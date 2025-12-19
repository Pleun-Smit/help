#include "Strategy/StaticStrategy.h"

void StaticStrategy::updateLogic(StationState& state)
{

}

float StaticStrategy::computePower(StationState& state)
{
    if (state.onlineStations <= 0)
    {
        return 0.0f;
    }

    float share = state.availablePower / state.onlineStations;

    if (share > 11.0f)
    {
        share = 11.0f;
    }

    return share;
}

const char* StaticStrategy::name() const 
{
    return "Static";
}