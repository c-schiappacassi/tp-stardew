#pragma once

#include "entity_id.h"

class EntityIdGenerator {
private:
    EntityId nextId;

public:
    explicit EntityIdGenerator(EntityId nextId = 1);

    EntityId generate();
    EntityId getNextId() const;
};