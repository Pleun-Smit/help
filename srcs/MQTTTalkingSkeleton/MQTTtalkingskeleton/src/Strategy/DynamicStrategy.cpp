#include "Strategy/DynamicStrategy.h"

void DynamicStrategy::updateLogic(StationState &state)
{
    (void)state;
}

float DynamicStrategy::computePower(StationState &state)
{
    if (state.onlineStations <= 0)
    {
        return 0.0f;
    }

    float AssignedPower = state.availablePower / state.onlineStations;

    if (AssignedPower > 11.0f)
    {
        AssignedPower = 11.0f;
    }

    return AssignedPower;

}
const char* DynamicStrategy::name() const
{
    return "Dynamic";
}