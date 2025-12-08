#pragma once
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <memory>
#include "StationState.h"
#include "SystemState.h"

class MQTTHandler {
public:
  MQTTHandler(const char* ssid,
              const char* password,
              const char* mqtt_server,
              uint16_t mqtt_port,
              std::shared_ptr<StationState> state,
              SystemState* systemState);

  void begin();
  void update();

  // Public publishing helpers
  void publishStatusToDashboard();           // station/{id}/status  (retained)
  void publishConnectionStatus(bool alive);  // station/{id}/connection (retained)

  // Utility publish
  void publish(String topic, String payload);
  void publish(String topic, String payload, int intervalMs);

private:
  WiFiClient wifi;
  PubSubClient client;
  const char* ssid;
  const char* password;
  const char* mqtt_server;
  uint16_t mqtt_port;
  std::shared_ptr<StationState> state;
  SystemState* systemState;

  static MQTTHandler* instance;
  static void callbackThunk(char*, byte*, unsigned int);
  void callback(char* topic, byte* payload, unsigned int length);

  void reconnect(); // sets MQTT Last Will to broadcast {"alive":false} on disconnect
  int  extractIdFromTopic(const String& topic);
};
