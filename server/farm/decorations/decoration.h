#pragma once

#include "dtos.h"
#include "entities/entity_id.h"
#include "position.h"
#include "protocol_types.h"

class Decoration {
private:
    EntityId id;
    Position position;
    ItemId item;
    Direction direction;

public:
    Decoration(EntityId id, const Position& position, ItemId item, Direction direction = Direction::Down);

    static bool isDecoration(ItemId item);
    EntityId getId() const;
    const Position& getPosition() const;
    ItemId getItem() const;
    Direction getDirection() const;
    bool blocksMovement() const;
    FurnitureData getPublicData() const;
};
