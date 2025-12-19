#pragma once
#include "IChargingStrategy.h"

class FCFSStrategy : public IChargingStrategy {
    public: 
        void updateLogic(StationState& state) override;

        float computePower(StationState& state) override;

        const char* name() const override;
};