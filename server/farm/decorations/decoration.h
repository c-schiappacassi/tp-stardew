#pragma once

#include "dtos.h"
#include "entities/collision_box.h"
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
    static CollisionBox collisionBoxFor(ItemId item, Direction direction);
    EntityId getId() const;
    const Position& getPosition() const;
    ItemId getItem() const;
    Direction getDirection() const;
    bool blocksMovement() const;
    CollisionBounds getBounds() const;
    FurnitureData getPublicData() const;
};
