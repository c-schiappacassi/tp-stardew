#pragma once

#include <cstdint>
#include <vector>

#include "farm/animals/animal_species.h"
#include "protocol_types.h"

struct FarmProductPrice {
    ItemId item;
    uint32_t basePrice;
};

struct FarmAnimalPrice {
    AnimalSpecies species;
    uint32_t basePrice;
};

struct FarmPrices {
    std::vector<FarmProductPrice> products;
    std::vector<FarmAnimalPrice> animals;
};
