#include "DynamicStrategy.h"


/**
 * @brief Dynamic allocation strategy implementation.
 *
 * Splits the available `totalpower` among the currently active stations
 * (`activestations`) and caps the per-station allocation at
 * `MaxPowerChargingStation`.
 *
 * @param totalpower Total available power to distribute.
 * @param availableStations Number of stations that are alive.
 * @param activestations Number of currently active/charging stations.
 * @param MaxPowerChargingStation Maximum allowed power per charging station.
 * @return Power allocated per active station according to the dynamic strategy.
 */
float DynamicStrategy::distributePower(float totalpower, int availableStations, int activestations, float MaxPowerChargingStation) 
{
    float returnpower = totalpower / activestations;
    if (returnpower > MaxPowerChargingStation)
    {
        returnpower = MaxPowerChargingStation;
    }
    return returnpower;
}
    