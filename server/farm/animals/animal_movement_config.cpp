#include "animal_movement_config.h"

#include "server_constants.h"

#include <stdexcept>

void AnimalMovementConfig::validate() const {
    if (stepPerTick <= 0 || decisionIntervalTicks == 0) {
        throw std::invalid_argument(server_constants::INVALID_MOVEMENT_STEP_OR_INTERVAL);
    }
}
