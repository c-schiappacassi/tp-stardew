#include "cow.h"

#include "server_constants.h"

#include <stdexcept>

Cow::Cow(EntityId id, const Position& position, AnimalType type,
    std::chrono::milliseconds productionInterval, const AnimalMovementConfig& movementConfig):
    Animal(id, position, type, productionInterval, movementConfig) {
    if (animalSpeciesForType(type) != AnimalSpecies::Cow) {
        throw std::invalid_argument(
            server_constants::THE_VARIANT_DOES_NOT_BELONG_TO_A_COW);
    }
}

AnimalSpecies Cow::getSpecies() const { return AnimalSpecies::Cow; }

ItemId Cow::getProduct() const { return ItemId::Milk; }

bool Cow::needsInteraction() const { return getAvailableProducts() > 0; }

void Cow::updateProduction(std::chrono::milliseconds elapsed) {
    if (!needsInteraction()) {
        advanceProduction(elapsed, ProductionMode::WaitForCollection);
    }
}
