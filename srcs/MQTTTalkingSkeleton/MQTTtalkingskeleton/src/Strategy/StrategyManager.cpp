#include "Strategy/StrategyManager.h"
#include "MQTTHandler/StationState.h"
#include "Strategy/StaticStrategy.h"
#include "Strategy/FCFSStrategy.h"

StrategyManager::StrategyManager(StationState& s) : state(s)
{   
    selectStrategy();
}

void StrategyManager::setMode(Mode newMode) 
{
    if (state.mode == newMode)
    {
        return;
    }

    state.mode = newMode;
    selectStrategy();
}

void StrategyManager::selectStrategy()
{
    switch (state.mode) {
        case Mode::Static:
            strategy.reset(new StaticStrategy());
            break;
        
        case Mode::FCFS:
            strategy.reset(new FCFSStrategy());
            break;

        case Mode::Dynamic:
        case Mode::Director:
        default:
            strategy.reset(new StaticStrategy());
            break;
    }
}

void StrategyManager::update()
{
    if (!strategy)
    {
        return;
    }

    strategy->updateLogic(state);
    state.allowedPower = strategy->computePower(state);
}

const char* StrategyManager::activeStrategyName() const 
{
    return strategy ? strategy->name() : "None";
}