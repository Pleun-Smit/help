#pragma once
#include <map>
#include <Arduino.h>
#include "StationState.h"

class SystemState {
private:
    struct StationInfo {
        StationState state;
        unsigned long lastUpdate;
    };

    std::map<int, StationInfo> stations;
    unsigned long timeoutMs;

public:
    SystemState(unsigned long timeout = 5000);
    void updateStation(int id, const String& mode, int power);
    void removeStation(int id);
    void checkTimeouts();
    void printData();
    const std::map<int, StationInfo>& getStations() const;
};
