#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include "animal_species.h"
#include "animal_movement_config.h"
#include "entities/collision_box.h"

struct AnimalDefinition {
    AnimalSpecies species;
    std::chrono::milliseconds productionInterval;
    std::size_t maxCount;
    uint32_t basePrice;
    uint32_t productPrice;
    AnimalMovementConfig movement{};
    CollisionBox collision;
};
