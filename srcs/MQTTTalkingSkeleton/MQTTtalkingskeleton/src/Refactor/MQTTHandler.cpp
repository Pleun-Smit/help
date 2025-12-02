#include "MQTTHandler.h"

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

void MQTTHandler::callback(char* topic, byte* payload, unsigned int length) {
    String t(topic);
    String msg;
    for (int i = 0; i < length; i++) msg += (char)payload[i];

    if (t.startsWith("station/") && (t.endsWith("/status") || t.endsWith("/connection"))) {
        int id = extractIdFromTopic(t);
        StaticJsonDocument<200> doc;
        if (deserializeJson(doc, msg)) return;

        bool alive = doc["alive"];
        String mode = doc["mode"] | "STATIC";
        int power = doc["power"] | 0;

        if (alive) {
            systemState->updateStation(id, mode, power);
        }
    } else if (t == "system/mode") {
        state->setMode(msg);
    }
}

void MQTTHandler::reconnect() {
    while (!client.connected()) {
        if (client.connect(("Station" + String(state->id)).c_str())) {
            client.subscribe("station/+/status");
            client.subscribe("station/+/connection");
            client.subscribe("system/mode");
        } else {
            delay(3000);
        }
    }
}

void MQTTHandler::publishStatusToDashboard() {
    static unsigned long lastPublish = 0;
    if (millis() - lastPublish > 2000) {
        lastPublish = millis();
        String payload = "{\"alive\":true,\"power\":" + String(state->power) + ",\"mode\":\"" + state->mode + "\"}";
        String topic = "station/" + String(state->id) + "/status";
        client.publish(topic.c_str(), payload.c_str(), true);
    }
}

void MQTTHandler::publishConnectionStatus() {
    static unsigned long lastPublish = 0;
    if (millis() - lastPublish > 1000) {
        lastPublish = millis();
        String payload = "{\"alive\":true,\"mode\":\"" + state->mode + "\"}";
        String topic = "station/" + String(state->id) + "/connection";
        client.publish(topic.c_str(), payload.c_str(), true);
    }
}

void MQTTHandler::update() {
    if (!client.connected()) reconnect();
    client.loop();
    publishStatusToDashboard();
    publishConnectionStatus();
    systemState->checkTimeouts();
}

void MQTTHandler::printData(){
    // probleem voor later
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
