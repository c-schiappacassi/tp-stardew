#include "chicken.h"

#include <stdexcept>

Chicken::Chicken(EntityId id, const Position& position, AnimalType type,
    std::chrono::milliseconds productionInterval, const AnimalMovementConfig& movementConfig):
    Animal(id, position, type, productionInterval, movementConfig) {
    if (animalSpeciesForType(type) != AnimalSpecies::Chicken) {
        throw std::invalid_argument("La variante no corresponde a una gallina");
    }
}

AnimalSpecies Chicken::getSpecies() const { return AnimalSpecies::Chicken; }

ItemId Chicken::getProduct() const { return ItemId::Egg; }

void Chicken::updateProduction(std::chrono::milliseconds elapsed) {
    advanceProduction(elapsed, ProductionMode::Accumulate);
}
