#pragma once
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "StationState.h"
#include "SystemState.h"

class MQTTHandler {
public:
    MQTTHandler(const char* ssid, const char* password, const char* mqtt_server,
                std::shared_ptr<StationState> state, SystemState* systemState);
    void begin();
    void update();


private:
    WiFiClient wifi;
    PubSubClient client;
    std::shared_ptr<StationState> state;
    SystemState* systemState;

    const char* ssid;
    const char* password;
    const char* mqtt_server;

    void printData();
    
    void publish(String topic, String payload);
    void publish(String topic, String payload, int interval);
    void reconnect();
    void publishStatusToDashboard();
    void publishConnectionStatus();
    static void callbackThunk(char*, byte*, unsigned int);
    void callback(char* topic, byte* payload, unsigned int length);
    int extractIdFromTopic(const String& topic);

    static MQTTHandler* instance;
};
