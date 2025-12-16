#ifndef STATIC_STRATEGY_H
#define STATIC_STRATEGY_H

#include "IPowerDistributionStrategy.h"


/**
 * @class StaticStrategy
 * @brief Static power distribution strategy.
 *
 * Distributes power in a fixed or evenly-split manner among charging stations.
 * This class implements `IPowerDistributionStrategy::distributePower`.
 */
class StaticStrategy : public IPowerDistributionStrategy {
public:
    /**
     * @brief Distribute the given input power according to the static strategy.
     *
     * @param inputPower Total power available to distribute.
     * @param availableStations Number of stations that are alive.
     * @param activestations Number of currently active/charging stations.
     * @param MaxPowerChargingStation Maximum allowed power per charging station.
     * @return Power allocated to a charging station according to this strategy.
     */
    float distributePower(float inputPower, int availableStations, int activestations, float MaxPowerChargingStation) override;
};

#endif // STATIC_STRATEGY_H