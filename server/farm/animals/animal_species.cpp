#include "animal_species.h"

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
