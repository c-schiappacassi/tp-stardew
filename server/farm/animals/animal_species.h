#pragma once

#include <optional>

#include "protocol_types.h"

enum class AnimalSpecies {
    Cow,
    Chicken
};

std::optional<AnimalSpecies> animalSpeciesForType(AnimalType type);
