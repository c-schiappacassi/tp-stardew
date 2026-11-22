#include "animal_species.h"

#include <array>

std::span<const AnimalType> animalTypesForSpecies(AnimalSpecies species) {
    static constexpr std::array cows{AnimalType::BrownCow, AnimalType::WhiteCow};
    static constexpr std::array chickens{AnimalType::BrownChicken, AnimalType::WhiteChicken,
                                        AnimalType::BlueChicken, AnimalType::VoidChicken};
    switch (species) {
        case AnimalSpecies::Cow: return cows;
        case AnimalSpecies::Chicken: return chickens;
    }
    return {};
}

std::optional<AnimalSpecies> animalSpeciesForType(AnimalType type) {
    switch (type) {
        case AnimalType::BrownCow:
        case AnimalType::WhiteCow:
            return AnimalSpecies::Cow;

        case AnimalType::BrownChicken:
        case AnimalType::WhiteChicken:
        case AnimalType::BlueChicken:
        case AnimalType::VoidChicken:
            return AnimalSpecies::Chicken;
    }

    return std::nullopt;
}
