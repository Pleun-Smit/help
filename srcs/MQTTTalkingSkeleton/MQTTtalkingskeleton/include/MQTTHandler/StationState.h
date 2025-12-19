#pragma once
#include <Arduino.h>

// Modes: STATIC, DYNAMIC, FCFS, DIRECTORS, SAFE
class StationState {
public:

  enum Mode {
      Static,
      Dynamic,
      FCFS,
      Director
  };

  int id;            // unique station id 0..N-1
  Mode mode;         // current operating mode
  int power;         // current local power draw (kW)
  bool charging;     // whether local output is enabled

  // Fields used by strategy manager and coordination
  float availablePower; // total available power reported for distribution
  int onlineStations;   // number of online stations participating
  float allowedPower;   // power allowed by strategy for this station
  int priorityIndex;

  StationState(): id(0), mode(Static), power(0), charging(false),
                  availablePower(0.0f), onlineStations(0), allowedPower(0.0f) {}
  StationState(int stationId, const Mode& defaultMode=Static, int defaultPower=0)
    : id(stationId), mode(defaultMode), power(defaultPower), charging(false),
      availablePower(0.0f), onlineStations(0), allowedPower(0.0f) {}

  void setMode(const Mode& newMode) { mode = newMode; }

};