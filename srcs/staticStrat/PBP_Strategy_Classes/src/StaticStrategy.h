#ifndef STATIC_STRATEGY_H
#define STATIC_STRATEGY_H

#include "IPowerDistributionStrategy.h"


class StaticStrategy : public IPowerDistributionStrategy {
public:
    float distributePower(float inputPower, int activestations, float MaxPowerChargingStation) override;
};

#endif // STATIC_STRATEGY_H