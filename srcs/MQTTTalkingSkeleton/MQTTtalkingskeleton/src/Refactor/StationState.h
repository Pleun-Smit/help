#pragma once
#include <Arduino.h>

class StationState {
public:
    int id;
    String mode;
    int power;

    StationState(int stationId)
        : id(stationId), mode("STATIC"), power(5) {}

    void setMode(const String& newMode) {
        mode = newMode;
    }
};
