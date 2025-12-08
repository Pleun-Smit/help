#include "SystemState.h"
#include <stdio.h>
#include <iostream>

SystemState::SystemState(unsigned long timeout) : timeoutMs(timeout) {}

void SystemState::updateStation(int id, const String& mode, int power, bool connected) {
    StationInfo info = { StationState(id), millis(), connected };
    info.state.setMode(mode);
    info.state.power = power;
    info.connected = connected;
    stations[id] = info;
}

void SystemState::removeStation(int id) {
    stations.erase(id);
}

void SystemState::checkTimeouts() {
    unsigned long now = millis();
    for (auto it = stations.begin(); it != stations.end(); ++it) {
        if (now - it->second.lastUpdate > timeoutMs) {
            it->second.connected = false;
        }
    }
}

int SystemState::aliveCount() const {
    int count = 0;
    for (const auto& pair : stations) {
        if (pair.second.connected && (millis() - pair.second.lastUpdate < timeoutMs))
            count++;
    }
    return count;
}

int SystemState::totalPower() const {
    int total = 0;
    for (const auto& pair : stations) {
        if (pair.second.connected)
            total += pair.second.state.power;
    }
    return total;
}

bool SystemState::allSameMode(String* modeOut) const {
    if (stations.empty()) return true;
    auto it = stations.begin();
    String firstMode = it->second.state.mode;
    if (modeOut) *modeOut = firstMode;
    for (++it; it != stations.end(); ++it) {
        if (it->second.state.mode != firstMode)
            return false;
    }
    return true;
}

String SystemState::majorityMode() const {
    std::map<String, int> modeCounts;
    for (const auto& pair : stations) {
        if (pair.second.connected) {
            modeCounts[pair.second.state.mode]++;
        }
    }
    int maxCount = 0;
    String major;
    for (const auto& kv : modeCounts) {
        if (kv.second > maxCount) {
            maxCount = kv.second;
            major = kv.first;
        }
    }
    if (major == "" || modeCounts.size() > 1) return "SAFE";
    return major;
}

void SystemState::printNeighborsToSerial(int selfId) const {
    Serial.println("---Neighbor Stations---");
    for (const auto& entry : stations) {
        Serial.print("ID: ");
        Serial.print(entry.first);
        Serial.print(" Mode: ");
        Serial.print(entry.second.state.mode);
        Serial.print(" Power: ");
        Serial.print(entry.second.state.power);
        Serial.print(" Connected: ");
        Serial.println(entry.second.connected ? "YES" : "NO");
    }
}

