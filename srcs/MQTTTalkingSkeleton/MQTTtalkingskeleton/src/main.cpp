#include "StationState.h"
#include "SystemState.h"
#include "MQTTHandler.h"
// 0 COM20, 1 COM19, 2 COM6, COM21 dash
// Set unique station ID for this ESP
constexpr int stationId = 2; // Change 0/1/2/3 for each device

auto station = std::make_shared<StationState>(stationId);
SystemState systemState(5000); // 5 second peer timeout window

MQTTHandler mqtt("EV-CHARGING", "TestTest", "192.168.137.1", station, &systemState);

void setup() {
    Serial.begin(115200);
    mqtt.initialize();
}

void loop() {
    mqtt.update();   // Handles MQTT + coordination logic
}