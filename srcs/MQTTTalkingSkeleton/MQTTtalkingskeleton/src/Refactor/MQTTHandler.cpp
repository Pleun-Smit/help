#include "MQTTHandler.h"

MQTTHandler* MQTTHandler::instance = nullptr;

MQTTHandler::MQTTHandler(const char* ssid, const char* password,
                        const char* mqtt_server, std::shared_ptr<StationState> state) : client(wifi), ssid(ssid), password(password), mqtt_server(mqtt_server), state(state) 
                        {
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

void MQTTHandler::callback(char* topic, byte* payload, unsigned int length) {
    String t(topic);
    String expected = "station/" + String(state->id) + "/mode";

    if (t == expected) {
        String msg;
        for (int i = 0; i < length; i++) msg += (char)payload[i];
        state->setMode(msg);
    }
    Serial.println(t);
}

void MQTTHandler::reconnect() {
    while (!client.connected()) {
        if (client.connect(("Station" + String(state->id)).c_str())) {
            String topic = "station/" + String(state->id) + "/mode";
            client.subscribe(topic.c_str());
        } else {
            delay(3000);
        }
    }
}

void MQTTHandler::publishStatus() {
    static unsigned long lastPublish = 0;
    if (millis() - lastPublish > 2000) {
        lastPublish = millis();
        String payload = "{\"alive\":true,\"power\":"
                         + String(state->power) +
                         ",\"mode\":\"" + state->mode + "\"}";
        String topic = "station/" + String(state->id) + "/status";
        client.publish(topic.c_str(), payload.c_str());
        Serial.println(payload);
    }
}

void MQTTHandler::update() {
    if (!client.connected()) reconnect();
    client.loop();
    publishStatus();
}
