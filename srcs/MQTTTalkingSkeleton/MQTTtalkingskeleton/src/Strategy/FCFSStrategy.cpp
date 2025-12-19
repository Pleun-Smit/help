#include "Strategy/FCFSStrategy.h"
#include <algorithm>

void FCFSStrategy::updateLogic(StationState& state)
{
    (void)state;
}

float FCFSStrategy::computePower(StationState& state)
{
    const float MAX_PER_STATION = 11.0f;

    if (state.availablePower <= 0.0f)
    {
        return 0.0f;
    }

    if (state.priorityIndex < 0)
    {
        return 0.0f;
    }

    float powerUsedBefore = state.priorityIndex * MAX_PER_STATION;

    float remainingPower = state.availablePower - powerUsedBefore;

    if (remainingPower <= 0.0f)
    {
        return 0.0f;
    }
    
    return std::min(remainingPower, MAX_PER_STATION);
}

const char* FCFSStrategy::name() const
{
    return "FCFS";
}