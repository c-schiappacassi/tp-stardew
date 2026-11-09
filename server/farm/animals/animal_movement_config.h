#pragma once

#include <cstdint>

class AnimalMovementConfig {
public:
    int stepPerTick = 1;
    uint32_t decisionIntervalTicks = 30;

    void validate() const;
};
