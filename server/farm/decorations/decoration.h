#pragma once

#include "entities/entity_id.h"
#include "position.h"
#include "protocol_types.h"

class Decoration {
private:
    EntityId id;
    Position position;
    ItemId item;

public:
    Decoration(EntityId id, const Position& position, ItemId item);

    static bool isDecoration(ItemId item);
    EntityId getId() const;
    const Position& getPosition() const;
    ItemId getItem() const;
    bool blocksMovement() const;
};
