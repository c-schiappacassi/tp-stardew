#pragma once

#include <cstddef>
#include <cstdint>
#include <chrono>
#include <vector>

#include "animals/animal_definition.h"
#include "crops/crop_definition.h"
#include "game_time.h"

class FarmConfig {
public:
    std::chrono::milliseconds dayLength;

    std::vector<CropDefinition> crops;
    std::vector<AnimalDefinition> animals;

    explicit FarmConfig(std::chrono::milliseconds dayLength = game_time::DAY_DURATION);

    void validate() const;
};
