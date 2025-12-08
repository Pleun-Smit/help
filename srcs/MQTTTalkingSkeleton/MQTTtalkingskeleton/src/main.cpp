#include "Refactor/StationState.h"
#include "Refactor/MQTTHandler.h"


auto station = std::make_shared<StationState>(1);
MQTTHandler mqtt("EV-CHARGING", "TestTest", "192.168.137.1", station);


void setup() {
    Serial.begin(115200);
    mqtt.begin();
}

void loop() {
    mqtt.update();   // Handles MQTT
}
