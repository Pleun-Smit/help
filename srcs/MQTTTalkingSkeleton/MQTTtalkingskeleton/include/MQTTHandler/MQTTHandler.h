#pragma once
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <memory>
#include "StationState.h"
#include "SystemState.h"
#include "Strategy/StrategyManager.h"

class MQTTHandler {
public:
    MQTTHandler(const char* ssid, const char* password, const char* mqtt_server,
                std::shared_ptr<StationState> state, SystemState* systemState);


  /**
   * Initializes MQTT, sets SSID, password and server
  */
  void initialize();

  /**
   * Updates the system, use in loop of main
   */
  void update();


  /**
   *  Publish a payload to a specific topic 
   */
  void publish(String topic, String payload);

  /**
   * Publish a payload to a specific topic every #Ms
   */
  void publish(String topic, String payload, int intervalMs);

  void ManualMode(StationState::Mode mode);

  void applyDashboardMode(const StationState::Mode& mode); 

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

  void publishStatusToDashboard();
  void publishConnectionStatus(bool alive);
  void printPeerInfo(int alivePeers, int totalPeers, bool neighborsMatch, StationState::Mode neighborMode);
  void printOwnInfo();
  
  void HandleSerialInput();


  unsigned long lastPrintTime = 0;
  const unsigned long interval = 2000;  // 2000ms

  unsigned long lastDashboardUpdate = 0;
  const unsigned long dashboardGraceMs = 5000; // 5000ms grace period

  std::unique_ptr<StrategyManager> strategyManager;
};