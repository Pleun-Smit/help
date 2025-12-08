#pragma once
#include <map>
#include <Arduino.h>
#include "StationState.h"

class SystemState {
public:
  struct StationInfo {
    StationState state;       // neighbor snapshot (id/mode/power)
    unsigned long lastUpdate; // millis() timestamp when we last saw a message
    bool connected;           // /connection topic says the peer is alive
  };

private:
  std::map<int, StationInfo> stations;
  unsigned long timeoutMs;

public:
  SystemState(unsigned long timeout = 5000);

  // Update or insert neighbor info. 'connected' defaults true for /status updates.
  void updateStation(int id, const String& mode, int power, bool connected = true);
  void removeStation(int id);
  void checkTimeouts();

  // --- Helpers for decentralized coordination ---
  int  aliveCount() const;                         // count of connected & recent neighbors
  int  totalPower() const;                         // sum of neighbors' power use
  bool allSameMode(String* modeOut = nullptr) const; // true if all neighbors share a mode
  String majorityMode() const;                     // majority vote; "SAFE" on tie/none

  // --- Serial output ---
  void printNeighborsToSerial(int selfId) const;

  const std::map<int, StationInfo>& getStations() const { return stations; }
};
