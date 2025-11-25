#pragma once
#include <WiFi.h>
#include <PubSubClient.h>
#include "StationState.h"

class MQTTHandler{
    public:
    MQTTHandler(const char* ssid, const char* password, const char* mqtt_server, std::shared_ptr<StationState>state);

    void begin();
    void update();

    private:
    WiFiClient wifi;
    PubSubClient client;
    std::shared_ptr<StationState> state;

    const char* ssid;
    const char* password;
    const char* mqtt_server;

    void reconnect();
    void publishStatus();
    static void callbackThunk(char*, byte*, unsigned int);
    void callback(char* topic, byte* payload, unsigned int length);

    static MQTTHandler* instance;
};