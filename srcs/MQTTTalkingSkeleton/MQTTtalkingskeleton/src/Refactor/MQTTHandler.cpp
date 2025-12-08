#include "MQTTHandler.h"

#define EXPECTED_STATION_COUNT 2    // Set this to the number of charging stations in your network

unsigned long lastPrintTime = 0;
const unsigned long interval = 2000;  // 500ms

unsigned long lastDashboardUpdate = 0;
const unsigned long dashboardGraceMs = 5000; // 1000ms grace period



MQTTHandler* MQTTHandler::instance = nullptr;

MQTTHandler::MQTTHandler(const char* ssid, const char* password, const char* mqtt_server,
                         std::shared_ptr<StationState> state, SystemState* systemState)
    : client(wifi), ssid(ssid), password(password), mqtt_server(mqtt_server), state(state), systemState(systemState) {
    instance = this;
}

void MQTTHandler::begin() {
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

// void MQTTHandler::callback(char* topic, byte* payload, unsigned int length) {
//     String t(topic);
//     String msg;
//     for (int i = 0; i < length; i++) msg += (char)payload[i];

//     if (t.startsWith("station/") && t.endsWith("/status")) {
//         int id = extractIdFromTopic(t);
//         StaticJsonDocument<200> doc;
//         if (deserializeJson(doc, msg)) return;
//         bool alive = doc["alive"];
//         String mode = doc["mode"] | "STATIC";
//         Serial.println("Rec mode:");
//         Serial.println(mode);
//         int power = doc["power"] | 0;
//         systemState->updateStation(id, mode, power, alive);
//     } else if (t.startsWith("station/") && t.endsWith("/connection")) {
//         int id = extractIdFromTopic(t);
//         StaticJsonDocument<200> doc;
//         if (deserializeJson(doc, msg)) return;
//         bool alive = doc["alive"];
//         String mode = doc["mode"] | "STATIC";
//         systemState->updateStation(id, mode, 0, alive); // connection only
//     } else if (t == "system/mode") {
//         state->setMode(msg);
//     }
// }

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
        StaticJsonDocument<50> doc;
        DeserializationError err = deserializeJson(doc, msg);
        if (!err) {
            String mode = doc["mode"] | "STATIC";
            state->setMode(mode);
            lastDashboardUpdate = millis();
            Serial.print("[MQTT] Dashboard mode applied: ");
            Serial.println(state->mode);
        } else {
            Serial.println("[MQTT] Failed to parse dashboard/mode JSON!");
        }
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

void MQTTHandler::update() {
    unsigned long now = millis();

    if (!client.connected()) reconnect();
    client.loop();

    publishStatusToDashboard();
    publishConnectionStatus(true);
    systemState->checkTimeouts();

    // Strongly enforce dashboard mode: if you ever received a dashboard/mode message, always keep that mode!
    static bool dashboardActive = false;
    if (lastDashboardUpdate != 0) // was a dashboard mode ever set in this session?
        dashboardActive = true;

    if (dashboardActive) {
        static String lastPrintedMode = "";
        if (state->mode != lastPrintedMode) {
            Serial.printf("[INFO] Dashboard mode is in force, keeping: %s\n", state->mode.c_str());
            lastPrintedMode = state->mode;
        }
        state->charging = true;
        return; // *** EARLY RETURN: never run the neighbor logic ***
    }

    // ------ Neighbor logic runs ONLY if no dashboard mode was ever set ------
    String neighborMode;
    bool neighborsMatch = systemState->allSameMode(&neighborMode);
    int alivePeers = systemState->aliveCount();
    int totalPeers = EXPECTED_STATION_COUNT;

    if (!neighborsMatch || alivePeers < totalPeers) {
        if (state->mode != "STATIC") {
            state->setMode("STATIC");
            Serial.println("[SAFETY] Entering STATIC mode due to failure/disagreement!");
        }
        state->charging = false;
    } else if (neighborMode != state->mode) {
        state->setMode(neighborMode);
        Serial.print("[SYNC] Neighbor mode differs → switching to: ");
        Serial.println(neighborMode);
        state->charging = true;
    } else {
        state->charging = true;  // all good
    }
}



// void MQTTHandler::update() {
//     unsigned long now = millis();

//     // MQTT connectivity check
//     if (!client.connected()) {
//         Serial.println("[MQTT] Client not connected, attempting reconnect...");
//         reconnect();
//     } else {
//         //Serial.println("[MQTT] Client connected.");
//     }

//     client.loop();
//     publishStatusToDashboard();
//     publishConnectionStatus(true);
//     systemState->checkTimeouts();

//     // --- Coordination logic ---
//     String neighborMode;
//     bool neighborsMatch = systemState->allSameMode(&neighborMode);
//     int alivePeers = systemState->aliveCount();
//     int totalPeers = EXPECTED_STATION_COUNT;

//     if (now - lastPrintTime >= interval) {
//         printPeerInfo(alivePeers, totalPeers, neighborMode, neighborMode);
//         lastPrintTime = now;
//     }

    

//     // If you have access to peer info, print each peer:
//     //systemState->printPeerTable();
//     //systemState->printNeighborsToSerial();  
//     // <--- You need to implement this if it doesn’t exist yet.
//     // It should print each station ID, last-heard timestamp, mode, etc.


//     // Logic for SAFE mode if peers are off or disagreement
//     // if (!neighborsMatch || alivePeers < totalPeers) {
//     //     Serial.println("[SAFETY] Issue detected:");
//     //     if (!neighborsMatch) Serial.println("   → Modes DO NOT MATCH!");
//     //     if (alivePeers < totalPeers) Serial.println("   → Some peers are offline!");

//     //     if (state->mode != "STATIC") {
//     //         Serial.println("[SAFETY] Entering SAFE (STATIC) mode!");
//     //         state->setMode("STATIC");
//     //     }

//     //     state->charging = false;
//     //     Serial.println("[SAFETY] Charging turned OFF.");
//     // } else {
//     //     Serial.println("[SYNC] All peers OK and matching.");

//     //     if (neighborMode != state->mode) {
//     //         Serial.print("[SYNC] Switching local mode to neighbor mode: ");
//     //         Serial.println(neighborMode);
//     //         state->setMode(neighborMode);
//     //     }

//     //     state->charging = true;
//     //     Serial.println("[SYNC] Charging ENABLED.");
//     // }

//     // Serial.println("----------------------------------------\n");
// }

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


// void MQTTHandler::update() {
//     if (!client.connected()) reconnect();
//     client.loop();
//     publishStatusToDashboard();
//     publishConnectionStatus(true);
//     systemState->checkTimeouts();

//     // --- Coordination logic ---
//     String neighborMode;
//     bool neighborsMatch = systemState->allSameMode(&neighborMode);
//     int alivePeers = systemState->aliveCount();
//     int totalPeers = EXPECTED_STATION_COUNT;

//     // Logic for SAFE mode if peers are off or disagreement
//     if (!neighborsMatch || alivePeers < totalPeers) {
//         if (state->mode != "STATIC") {
//             state->setMode("STATIC");
//             Serial.println("[SAFETY] Entering SAFE mode due to failure/disagreement!");
//         }
//         // Optionally: Stop charging, set local output off
//         state->charging = false;
//     } else {
//         // If local mode disagrees with majority, synchronize
//         if (neighborMode != state->mode) {
//             state->setMode(neighborMode);
//             Serial.print("[SYNC] Switching mode to ");
//             Serial.println(neighborMode);
//         }
//         // Optionally: Start/resume charging if appropriate for mode
//         state->charging = true;  // Or your internal logic
//     }
// }

// void MQTTHandler::printData(){
//     systemState->printNeighborsToSerial(state->id);
// }

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