#include "animal_factory.h"

#include "chicken.h"
#include "cow.h"

std::unique_ptr<Animal> AnimalFactory::create(EntityId id, const Position& position,
    AnimalType type, const AnimalDefinition& definition) {
    if (animalSpeciesForType(type) != definition.species) {
        return nullptr;
    }

    switch (definition.species) {
        case AnimalSpecies::Cow:
            return std::make_unique<Cow>(id, position, type, definition.productionInterval,
                definition.movement);

        case AnimalSpecies::Chicken:
            return std::make_unique<Chicken>(id, position, type, definition.productionInterval,
                definition.movement);
    }

    return nullptr;
}
