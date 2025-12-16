#include "MQTTHandler.h"

#define EXPECTED_STATION_COUNT 2    // Set this to the number of charging stations in your network



MQTTHandler* MQTTHandler::instance = nullptr;

MQTTHandler::MQTTHandler(const char* ssid, const char* password, const char* mqtt_server,
                         std::shared_ptr<StationState> state, SystemState* systemState)
    : client(wifi), ssid(ssid), password(password), mqtt_server(mqtt_server), state(state), systemState(systemState) {
    instance = this;
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
        int id = extractIdFromTopic(t);
        StaticJsonDocument<200> doc;
        if (deserializeJson(doc, msg)) return;

        bool alive = doc["alive"];
        String mode = doc["mode"] | "STATIC";
        int power = doc["power"] | 0;

        systemState->updateStation(id, mode, power, alive);

        Serial.printf("[MQTT] Station %d status: mode=%s, power=%d, alive=%s\n",
                      id, mode.c_str(), power, alive ? "true" : "false");
    }

    // ------------------ Dashboard mode ------------------
    else if (t == "dashboard/mode") {
        StaticJsonDocument<64> doc;
        DeserializationError err = deserializeJson(doc, msg);
        if (err) {
            Serial.println("[MQTT] Failed to parse dashboard/mode JSON!");
        } else {
            String mode = doc["mode"] | "STATIC";

            // 1) Apply locally
            state->setMode(mode);

            // 2) Start dashboard grace period
            lastDashboardUpdate = millis();

            // 3) Publish immediately to peers
            String payload = "{\"alive\":true,\"power\":" + String(state->power) +
                             ",\"mode\":\"" + state->mode + "\"}";
            String topicOut = "station/" + String(state->id) + "/status";
            client.publish(topicOut.c_str(), payload.c_str(), true);

            Serial.printf("[MQTT] Dashboard mode applied and published: %s\n", mode.c_str());
        }
    }
    else {
        Serial.printf("Topic is illegal");
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

        String payload = "{\"alive\":true,\"power\":" + String(state->power) +
                         ",\"mode\":\"" + state->mode + "\"}";

        String topic = "station/" + String(state->id) + "/status";
        Serial.printf("[PUBLISH] topic='%s', payload=%s\n", topic.c_str(), payload.c_str());
        client.publish(topic.c_str(), payload.c_str(), true);
    }
}

void MQTTHandler::publishConnectionStatus(bool alive) {
    static unsigned long lastPublish = 0;
    if (millis() - lastPublish > 1000) {
        lastPublish = millis();
        String payload = "{\"alive\":" + String(alive ? "true" : "false") + ",\"mode\":\"" + state->mode + "\"}";
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

    if (dashboardInGrace) {
        static unsigned long lastPrint = 0;
        if (now - lastPrint > 1000) {
            unsigned long remaining = dashboardGraceMs - (now - lastDashboardUpdate);
            Serial.printf("[INFO] Dashboard override active (%lu ms left), mode=%s\n",
                          remaining, state->mode.c_str());
            lastPrint = now;
        }

        state->charging = true; // allow charging
        return;                 // skip neighbor/safety logic
    }

    // ---------------- Neighbor / safety logic (only after grace) ----------------
    String neighborMode;
    bool neighborsMatch = systemState->allSameMode(&neighborMode);
    int alivePeers = systemState->aliveCount();
    int totalPeers = EXPECTED_STATION_COUNT;

    // Periodic debug
    if (now - lastPrintTime > interval) {
        printPeerInfo(alivePeers, totalPeers, neighborsMatch, neighborMode);
        lastPrintTime = now;
    }

    bool disagreement = !neighborsMatch;
    bool missingPeers = alivePeers < totalPeers;

    if (disagreement || missingPeers) {
        if (state->mode != "STATIC") {
            Serial.println("[SAFETY] Disagreement or offline peers → forcing STATIC mode.");
            state->setMode("STATIC");
        }
        state->charging = false;
        return;
    }

    // Sync mode if needed
    if (neighborMode != state->mode) {
        Serial.printf("[SYNC] Sync local mode to %s\n", neighborMode.c_str());
        state->setMode(neighborMode);
    }

    state->charging = true; // normal operation
}


void MQTTHandler::printPeerInfo(int alivePeers, int totalPeers, bool neighborsMatch, String neighborMode){
    Serial.println("---------- Coordination Debug ----------");
    Serial.print("[DEBUG] Alive peers: ");
    Serial.print(alivePeers);
    Serial.print(" / ");
    Serial.println(totalPeers);

    Serial.print("[DEBUG] Neighbors match: ");
    Serial.println(neighborsMatch ? "YES" : "NO");

    Serial.print("[DEBUG] Neighbor selected mode: ");
    Serial.println(neighborMode);

    Serial.print("[DEBUG] Local mode: ");
    Serial.println(state->mode);

    Serial.print("[DEBUG] Local charging: ");
    Serial.println(state->charging ? "ON" : "OFF");
}


void MQTTHandler::printOwnInfo(){
    Serial.println("Mode: ");
    Serial.println(state->mode);
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

void MQTTHandler::ManualMode(String mode){
    state->setMode(mode);
}

void MQTTHandler::applyDashboardMode(const String& mode) {
    state->setMode(mode);
    lastDashboardUpdate = millis();
    publishStatusToDashboard();  // Force immediate publish
}


void MQTTHandler::HandleSerialInput(){
    if(Serial.available()){
        String mode = Serial.readString();
        ManualMode(mode);
    }
}   