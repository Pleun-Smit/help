#pragma once
#include <Arduino.h>

// Modes: STATIC, DYNAMIC, FCFS, DIRECTORS, SAFE
class StationState {
public:
  int id;            // unique station id 0..N-1
  String mode;       // current operating mode
  int power;         // current local power draw (kW)
  bool charging;     // whether local output is enabled

  StationState(): id(0), mode("STATIC"), power(0), charging(false) {}
  StationState(int stationId, const String& defaultMode="STATIC", int defaultPower=0)
    : id(stationId), mode(defaultMode), power(defaultPower), charging(false) {}

  void setMode(const String& newMode) { mode = newMode; }
};
