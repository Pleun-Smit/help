#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <map>

// These maps are defined in your main .ino file
extern std::map<String, bool> deviceOnline;
extern std::map<String, unsigned long> deviceLastSeen;

class MQTTManager {
public:
    MQTTManager(Client& netClient, const char* mqttServer, int mqttPort);

    void begin();
    void loop();
    void subscribePresence(const String& deviceName);

private:
    PubSubClient mqttClient;
    const char* server;
    int port;

    void callback(char* topic, byte* payload, unsigned int length);
    void reconnect();
};

#endif
