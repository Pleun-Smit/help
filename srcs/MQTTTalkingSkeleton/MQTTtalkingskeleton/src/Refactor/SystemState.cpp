#include "SystemState.h"
#include <stdio.h>
#include <iostream>


SystemState::SystemState(unsigned long timeout) : timeoutMs(timeout) {}

void SystemState::updateStation(int id, const String& mode, int power, bool connected) {
    StationInfo info = { StationState(id), millis() };
    info.state.setMode(mode);
    info.state.power = power;
    stations[id] = info;
}

void SystemState::removeStation(int id) {
    stations.erase(id);
}

void SystemState::checkTimeouts() {
    unsigned long now = millis();
    for (auto it = stations.begin(); it != stations.end();) {
        if (now - it->second.lastUpdate > timeoutMs) {
            it = stations.erase(it);
        } else {
            ++it;
        }
    }
}

// void SystemState::printData(){
//     for(const auto& enrty : stations){
//         std::cout << " " << enrty.first << "\n";
//     }
// }

const std::map<int, SystemState::StationInfo>& SystemState::getStations() const {
    return stations;
}
