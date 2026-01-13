#include "Strategy/StrategyManager.h"
#include "MQTTHandler/StationState.h"
#include "Strategy/StaticStrategy.h"
#include "Strategy/FCFSStrategy.h"
#include "Strategy/DynamicStrategy.h"

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
            Serial.println("STATIC strat");
            break;
        
        case Mode::FCFS:
            strategy.reset(new FCFSStrategy());
            Serial.println("[STRAT MANAGER] FCFS strat");
            break;

        case Mode::Dynamic:
            strategy.reset(new DynamicStrategy());
            Serial.println("[STRAT MANAGER] Dynamic strat");
            break;
        case Mode::Director:
            Serial.println("[STRAT MANAGER] DIR strat");
            break;
        default:
            strategy.reset(new StaticStrategy());
            Serial.println("[STRAT MANAGER] Default");
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
    state.power = state.allowedPower;
}

const char* StrategyManager::activeStrategyName() const 
{
    return strategy ? strategy->name() : "None";
}