#include "MQTTHandler/MQTTHandler.h"
#include "MQTTHandler/Helpers.h"
#include "Strategy/StrategyManager.h"

#define EXPECTED_STATION_COUNT 4    // Set this to the number of charging stations in network




MQTTHandler* MQTTHandler::instance = nullptr;

MQTTHandler::MQTTHandler(const char* ssid, const char* password, const char* mqtt_server,
                         std::shared_ptr<StationState> state, SystemState* systemState)
    : client(wifi), ssid(ssid), password(password), mqtt_server(mqtt_server), state(state), systemState(systemState) {
    instance = this;

    // Create and own a StrategyManager that works on the shared StationState
    // NOTE: StrategyManager expects a StationState&; dereference the shared_ptr
    //strategyManager = std::make_unique<StrategyManager>(*state);
    strategyManager = std::unique_ptr<StrategyManager>(new StrategyManager(*state));

}

void MQTTHandler::initialize() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) delay(500);

    client.setServer(mqtt_server, 1883);
    client.setCallback(callbackThunk);
}

void MQTTHandler::callbackThunk(char* topic, byte* payload, unsigned int length) {
    if (instance) instance->callback(topic, payload, length);
}

int MQTTHandler::extractIdFromTopic(const String& topic) {
    int firstSlash = topic.indexOf('/') + 1;
    int secondSlash = topic.indexOf('/', firstSlash);
    return topic.substring(firstSlash, secondSlash).toInt();
}

// ---------------- Dashboard callback ----------------
void MQTTHandler::callback(char* topic, byte* payload, unsigned int length) {
    String t(topic);
    String msg;
    for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
    msg.trim();

    // ------------------ Station status ------------------
    if (t.startsWith("station/") && t.endsWith("/status")) {
        StaticJsonDocument<200> doc;
        DeserializationError err = deserializeJson(doc, msg);
        if (err) {
            Serial.println("[MQTT] Failed to parse station status JSON!");
            return;
        }

        int id = extractIdFromTopic(t);
        bool alive = doc["alive"] | false;
        String modeStr = doc["mode"] | "STATIC";
        StationState::Mode mode = Helpers::modeFromString(modeStr);
        int power = doc["power"] | 0;

        systemState->updateStation(id, mode, power, alive);

        Serial.printf("[MQTT] Station %d status: mode=%s, power=%d, alive=%s\n",
                      id, modeStr.c_str(), power, alive ? "true" : "false");
        return;
    }

    // ------------------ Dashboard mode ------------------
    else if (t == "dashboard/mode") {
        StaticJsonDocument<64> doc;
        DeserializationError err = deserializeJson(doc, msg);
        if (err) {
            Serial.println("[MQTT] Failed to parse dashboard/mode JSON!");
            return;
        } else {
            String mode = doc["mode"] | "STATIC";
            StationState::Mode mode2 = Helpers::modeFromString(mode);

            // 1) Apply locally
            state->setMode(mode2);

            // 2) Start dashboard grace period
            lastDashboardUpdate = millis();

            // 3) Recompute strategy before immediate publish so allowedPower is up-to-date
            if (strategyManager) {
                strategyManager->update();
            }

            // 4) Publish immediately to peers (now include allowedPower)
            String payload = "{\"alive\":true,\"power\":" + String(state->power) +
                             ",\"allowedPower\":" + String(state->allowedPower, 2) +
                             ",\"mode\":\"" + Helpers::modeToString(state->mode) + "\"}";
            String topicOut = "station/" + String(state->id) + "/status";
            client.publish(topicOut.c_str(), payload.c_str(), true);

            Serial.printf("[MQTT] Dashboard mode applied and published: %s\n", mode.c_str());
            return;
        }
    }

    // ------------------ System mode (example handler) ------------------
    else if (t == "system/mode") {
        Serial.printf("[MQTT] system/mode: %s\n", msg.c_str());
        return;
    }

    // ------------------ Station connection ------------------
    else if (t.startsWith("station/") && t.endsWith("/connection")) {
        Serial.printf("[MQTT] station connection: %s\n", t.c_str());
        return;
    }

    else {
        Serial.printf("[MQTT] Topic is illegal: %s\n", t.c_str());
        return;
    }
}

void MQTTHandler::reconnect() {
    while (!client.connected()) {
        if (client.connect(("Station" + String(state->id)).c_str())) {
            client.subscribe("station/+/status");
            client.subscribe("station/+/connection");
            client.subscribe("dashboard/mode");
            client.subscribe("system/mode");
        } else {
            Serial.printf("Unable to reconnect... trying again later");
            delay(3000);
        }
    }
}

// ------------------ Publish station status ------------------
void MQTTHandler::publishStatusToDashboard() {
    static unsigned long lastPublish = 0;
    if (millis() - lastPublish > 2000) {
        lastPublish = millis();

        // Make sure allowedPower is current before we publish
        if (strategyManager) {
            strategyManager->update();
        }

        String payload = "{\"alive\":true"
                 + String(",\"power\":") + String(state->power)
                 + String(",\"allowedPower\":") + String(state->allowedPower, 2)
                 + String(",\"mode\":\"") + Helpers::modeToString(state->mode) + "\"}";

        String topic = "station/" + String(state->id) + "/status";
        Serial.printf("[PUBLISH] topic='%s', payload=%s\n", topic.c_str(), payload.c_str());
        client.publish(topic.c_str(), payload.c_str(), true);
    }
}

void MQTTHandler::publishConnectionStatus(bool alive) {
    static unsigned long lastPublish = 0;
    if (millis() - lastPublish > 1000) {
        lastPublish = millis();
        String payload = "{\"alive\":" + String(alive ? "true" : "false") + ",\"mode\":\"" + Helpers::modeToString(state->mode) + "\"}";
        String topic = "station/" + String(state->id) + "/connection";
        client.publish(topic.c_str(), payload.c_str(), true);
    }
}


// ---------------- Update loop ----------------
void MQTTHandler::update() {
    unsigned long now = millis();

    if (!client.connected()) reconnect();
    client.loop();

    // Routine housekeeping
    publishStatusToDashboard();
    publishConnectionStatus(true);
    systemState->checkTimeouts();
    HandleSerialInput();

    // ---------------- Dashboard grace check (first!) ----------------
    bool dashboardInGrace = (lastDashboardUpdate != 0) &&
                            ((now - lastDashboardUpdate) < dashboardGraceMs);
    String modeStr = Helpers::modeToString(state->mode);

    if (dashboardInGrace) {
        static unsigned long lastPrint = 0;
        if (now - lastPrint > 1000) {
            unsigned long remaining = dashboardGraceMs - (now - lastDashboardUpdate);
            Serial.printf("[INFO] Dashboard override active (%lu ms left), mode=%s\n",
                          remaining, modeStr.c_str());
            lastPrint = now;
        }

        state->charging = true; // allow charging

        if (strategyManager) strategyManager->update();

        return;                 // skip neighbor/safety logic
    }

    // ---------------- Neighbor / safety logic (only after grace) ----------------
    StationState::Mode neighborMode;
    bool neighborsMatch = systemState->allSameMode(&neighborMode);
    int alivePeers = systemState->aliveCount();
    int totalPeers = EXPECTED_STATION_COUNT;

    // Periodic debug
    if (now - lastPrintTime > interval) {
        printPeerInfo(alivePeers, totalPeers, neighborsMatch, neighborMode);
        lastPrintTime = now;
    }

    bool disagreement = !neighborsMatch || (neighborMode != state->mode);
    bool missingPeers = alivePeers < totalPeers;

    if (disagreement) { // || missingPeers
        if (state->mode != StationState::Static) {
            Serial.println("[SAFETY] Disagreement or offline peers, forcing STATIC mode.");
            state->setMode(StationState::Static);
        }
        state->charging = false;
        if (strategyManager) strategyManager->update();
        return;
    }

    String neighborModeStr = Helpers::modeToString(neighborMode);
    // Sync mode if needed
    if (neighborMode != state->mode) {
        Serial.printf("[SYNC] Sync local mode to %s\n", neighborModeStr.c_str());
        state->setMode(neighborMode);
    }

    state->charging = true; // normal 
    
    state->availablePower = 20; /* calculate or read available power */ //state->availablePower; // need this from the building
    state->onlineStations = alivePeers; // include this station was +1 but noticed that that was incorrect

    if (strategyManager) {
        strategyManager->update();
    }
}


void MQTTHandler::printPeerInfo(int alivePeers, int totalPeers, bool neighborsMatch, StationState::Mode neighborMode){
    Serial.println("---------- Coordination Debug ----------");
    Serial.print("[DEBUG] Alive peers: ");
    Serial.print(alivePeers);
    Serial.print(" / ");
    Serial.println(totalPeers);

    Serial.print("[DEBUG] Neighbors match: ");
    Serial.println(neighborsMatch ? "YES" : "NO");

    Serial.print("[DEBUG] Neighbor selected mode: ");
    Serial.println(Helpers::modeToString(neighborMode));

    Serial.print("[DEBUG] Local mode: ");
    Serial.println(Helpers::modeToString(state->mode));

    Serial.print("[DEBUG] Local charging: ");
    Serial.println(state->charging ? "ON" : "OFF");
}


void MQTTHandler::printOwnInfo(){
    Serial.println("Mode: ");
    Serial.println(Helpers::modeToString(state->mode));
}

void MQTTHandler::publish(String topic, String payload){
    client.publish(topic.c_str(), payload.c_str(), true);
}

void MQTTHandler::publish(String topic, String payload, int interval){
    static unsigned long lastPublish = 0;
    if (millis() - lastPublish > interval) {
        lastPublish = millis();
        client.publish(topic.c_str(), payload.c_str(), true);
    }
}

void MQTTHandler::ManualMode(StationState::Mode mode){
    state->setMode(mode);
}

void MQTTHandler::applyDashboardMode(const StationState::Mode& mode) {
    state->setMode(mode);
    lastDashboardUpdate = millis();
    publishStatusToDashboard();  // Force immediate publish
}


void MQTTHandler::HandleSerialInput(){
    if(Serial.available()){
        String modeStr = Serial.readString();
        StationState::Mode mode = Helpers::modeFromString(modeStr);
        ManualMode(mode);
    }
}