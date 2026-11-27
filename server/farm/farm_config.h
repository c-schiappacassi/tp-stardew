#pragma once

#include <cstddef>
#include <cstdint>
#include <chrono>
#include <optional>
#include <vector>

#include "animals/animal_definition.h"
#include "crops/crop_definition.h"
#include "game_time.h"
#include "queries/farm_prices.h"

class FarmConfig {
public:
    std::chrono::milliseconds dayLength;

    std::vector<CropDefinition> crops;
    std::vector<AnimalDefinition> animals;

    explicit FarmConfig(std::chrono::milliseconds dayLength = game_time::DAY_DURATION);

    void validate() const;
    std::optional<uint32_t> animalBasePrice(AnimalSpecies species) const;
    std::optional<uint32_t> productBasePrice(ItemId item) const;
    FarmPrices prices() const;

private:
    static void validateDayLength(std::chrono::milliseconds dayLength);
};
