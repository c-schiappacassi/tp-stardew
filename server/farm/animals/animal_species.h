#pragma once

#include <optional>
#include <span>

#include "protocol_types.h"

enum class AnimalSpecies {
    Cow,
    Chicken
};

std::optional<AnimalSpecies> animalSpeciesForType(AnimalType type);
std::span<const AnimalType> animalTypesForSpecies(AnimalSpecies species);
