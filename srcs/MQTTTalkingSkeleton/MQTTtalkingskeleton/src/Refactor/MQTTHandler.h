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
              std::shared_ptr<StationState> state,
              SystemState* systemState);

  void begin();
  void update();

  void publishStatusToDashboard();
  void publishConnectionStatus(bool alive);
  void printPeerInfo(int alivePeers, int totalPeers, bool neighborsMatch, String neighborMode);
  void printOwnInfo();

  void publish(String topic, String payload);
  void publish(String topic, String payload, int intervalMs);

private:
  WiFiClient wifi;
  PubSubClient client;
  const char* ssid;
  const char* password;
  const char* mqtt_server;
  std::shared_ptr<StationState> state;
  SystemState* systemState;

  static MQTTHandler* instance;
  static void callbackThunk(char*, byte*, unsigned int);
  void callback(char* topic, byte* payload, unsigned int length);

  void reconnect();
  int  extractIdFromTopic(const String& topic);
};