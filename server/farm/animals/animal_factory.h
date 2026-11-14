#pragma once

#include <memory>

#include "animal.h"
#include "animal_definition.h"

class AnimalFactory {
public:
    static std::unique_ptr<Animal> create(EntityId id, const Position& position, AnimalType type,const AnimalDefinition& definition);
};
