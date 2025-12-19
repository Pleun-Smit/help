#pragma once
#include <Arduino.h>
#include "StationState.h"

class Helpers{
    public:
        static StationState::Mode modeFromString(const String& s);

        static String modeToString(StationState::Mode m);

    private:

};