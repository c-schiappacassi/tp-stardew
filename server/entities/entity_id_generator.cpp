#include "entity_id_generator.h"

#include <stdexcept>

EntityIdGenerator::EntityIdGenerator(EntityId nextId):nextId(nextId) {}

EntityId EntityIdGenerator::generate() {
    if (nextId == 0) {
        throw std::overflow_error("Game entity identifiers exhausted");
    }
    return nextId++;
}

EntityId EntityIdGenerator::getNextId() const {
    return nextId;
}
void EntityIdGenerator::ensureNextId(EntityId minimum) {
    if (nextId != 0 && nextId < minimum) nextId = minimum;
}
