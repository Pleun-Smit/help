#include "Helpers.h"
#include "StationState.h"


static StationState::Mode modeFromString(const String& s) {
        if (s == "DYNAMIC")   return StationState::Dynamic;
        if (s == "FCFS")      return StationState::FCFS;
        if (s == "DIRECTOR")  return StationState::Director;
        return StationState::Static; // default / safety
    }

static String modeToString(StationState::Mode m) {
        switch (m) {
            case StationState::Dynamic:  return "DYNAMIC";
            case StationState::FCFS:     return "FCFS";
            case StationState::Director: return "DIRECTOR";
            default:       return "STATIC";
        }
    }