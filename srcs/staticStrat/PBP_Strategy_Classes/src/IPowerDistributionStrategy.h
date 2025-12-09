#pragma once
#include <vector>

class IPowerDistributionStrategy {
public:
    virtual ~IPowerDistributionStrategy() = default;

    // Distributes the input power into multiple output channels.
    virtual float distributePower(float totalPower,  int activestations, float MaxPowerChargingStation) = 0;
};
