#pragma once
#include <map>
#include <Arduino.h>
#include "StationState.h"

class SystemState {
  public:
  struct StationInfo {
    StationState state;       // neighbor snapshot (id/mode/power)
    unsigned long lastUpdate; // millis() when last update seen
    bool connected;           // true if /connection or recent /status
    StationInfo() : state(), lastUpdate(0), connected(false) {}
    StationInfo(StationState s, unsigned long t, bool c = false)
      : state(s), lastUpdate(t), connected(c) {}
  };
  SystemState(unsigned long timeout = 5000);

  void updateStation(int id, const String& mode, int power, bool connected = true);
  void removeStation(int id);

  void checkTimeouts();

  // --- Coordination helpers ---
  int  aliveCount() const;                         // stations seen and marked connected
  int  totalPower() const;                         // sum of peer stations with connected = true
  bool allSameMode(String* modeOut = nullptr) const; // true if all reported modes match
  String majorityMode() const;                     // majority mode ("SAFE" on tie)

  void printNeighborsToSerial(int selfId) const;

  const std::map<int, StationInfo>& getStations() const { return stations; }

private:
  std::map<int, StationInfo> stations;
  unsigned long timeoutMs;


};