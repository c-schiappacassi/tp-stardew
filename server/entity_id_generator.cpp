#include "entity_id_generator.h"

#include <stdexcept>

EntityIdGenerator::EntityIdGenerator(EntityId nextId):nextId(nextId) {}

EntityId EntityIdGenerator::generate() {
    if (nextId == 0) {
        throw std::overflow_error("Se agotaron los identificadores de la partida");
    }
    return nextId++;
}

EntityId EntityIdGenerator::getNextId() const {
    return nextId;
}