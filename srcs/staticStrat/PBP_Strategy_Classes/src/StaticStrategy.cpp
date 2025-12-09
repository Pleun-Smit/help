#include "StaticStrategy.h"


/**
 * @brief Static allocation strategy implementation.
 *
 * Distributes the `totalpower` evenly across the `availableStations` and
 * enforces the `MaxPowerChargingStation` cap for any per-station allocation.
 *
 * @param totalpower Total available power to distribute.
 * @param availableStations Number of stations that are alive.
 * @param activestations Number of currently active/charging stations.
 * @param MaxPowerChargingStation Maximum allowed power per charging station.
 * @return Power allocated per station according to the static strategy.
 */
float StaticStrategy::distributePower(float totalpower, int availableStations, int activestations, float MaxPowerChargingStation) 
{
    float returnpower = totalpower / availableStations;
    if (returnpower > MaxPowerChargingStation)
    {
        returnpower = MaxPowerChargingStation;
    }
    return returnpower;
}
    