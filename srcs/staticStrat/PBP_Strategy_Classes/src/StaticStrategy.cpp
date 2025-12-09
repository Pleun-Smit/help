#include "StaticStrategy.h"

float StaticStrategy::distributePower(float totalpower, int activestations, float MaxPowerChargingStation) 
{
    float returnpower = totalpower / activestations;
    if (returnpower > MaxPowerChargingStation)
    {
        returnpower = MaxPowerChargingStation;
    }
    return returnpower;
}
    