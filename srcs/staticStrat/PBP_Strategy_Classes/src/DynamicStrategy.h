#ifndef DYNAMIC_STRATEGY_H
#define DYNAMIC_STRATEGY_H

#include "IPowerDistributionStrategy.h"

class DynamicStrategy : public IPowerDistributionStrategy {
public:
    /**
     * @class DynamicStrategy
     * @brief Dynamic power distribution strategy.
     *
     * Implements a dynamic algorithm that adapts the power allocation based
     * on runtime conditions (e.g., number of active stations, available
     * capacity, and per-station maximums). The exact behavior is provided
     * by the implementation in the corresponding source file.
     */

    /**
     * @brief Calculate a dynamic power allocation.
     *
     * @param inputPower Total available power to distribute.
     * @param availableStations Number of stations that are alive.
     * @param activestations Number of currently active/charging stations.
     * @param MaxPowerChargingStation Maximum allowed power per charging station.
     * @return Amount of power to allocate according to the dynamic strategy.
     */
    float distributePower(float inputPower, int availableStations, int activestations, float MaxPowerChargingStation) override;
};

#endif // Dynamic_STRATEGY_H