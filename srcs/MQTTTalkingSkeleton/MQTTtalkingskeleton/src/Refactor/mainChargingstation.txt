#include "Refactor/StationState.h"
#include "Refactor/SystemState.h"
#include "Refactor/MQTTHandler.h"

// Set unique station ID for this ESP
constexpr int stationId = 2; // Change 0/1/2/3 for each device

auto station = std::make_shared<StationState>(stationId);
SystemState systemState(5000); // 5 second peer timeout window

MQTTHandler mqtt("EV-CHARGING", "TestTest", "192.168.137.1", station, &systemState);

void setup() {
    Serial.begin(115200);
    mqtt.begin();
}

void loop() {
    mqtt.update();   // Handles MQTT + coordination logic
}