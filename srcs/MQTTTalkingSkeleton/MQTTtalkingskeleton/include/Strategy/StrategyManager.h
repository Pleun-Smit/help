#pragma once
#include <memory>
#include "MQTTHandler/StationState.h"
#include "IChargingStrategy.h"

class StrategyManager {
public:
    using Mode = StationState::Mode;
    
    explicit StrategyManager(StationState& s);

    void update();

    void setMode(Mode newMode);
    const char* activeStrategyName() const;

private:
    StationState& state;
    std::unique_ptr<IChargingStrategy> strategy;

    void selectStrategy();
};
