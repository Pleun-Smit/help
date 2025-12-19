#pragma once
#include "MQTTHandler/StationState.h"

class IChargingStrategy {
    public:
        virtual ~IChargingStrategy() {}

        virtual void updateLogic(StationState& state) = 0;

        virtual float computePower(StationState& state) = 0;

        virtual const char* name() const = 0;
};