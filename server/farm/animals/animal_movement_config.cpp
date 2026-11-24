#include "animal_movement_config.h"

#include <stdexcept>

void AnimalMovementConfig::validate() const {
    if (stepPerTick <= 0 || decisionIntervalTicks == 0) {
        throw std::invalid_argument("Invalid movement step or interval");
    }
}
