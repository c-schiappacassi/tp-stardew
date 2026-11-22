#pragma once

#include <optional>
#include "entities/collision_box.h"
#include "entities/entity_id.h"
#include "world/zone_map.h"

class Placement {
public:
    virtual ~Placement();
    virtual std::optional<ZoneMap::Cell> cellAt(const Position& base) const = 0;
    virtual bool contains(EntityId id) const = 0;
    virtual bool isAvailable(const Position& base, EntityId movingId) const = 0;
    virtual bool isAvailable(const CollisionBounds& bounds, EntityId ignoredId = 0) const = 0;
    virtual bool reserve(EntityId id, const Position& base, const CollisionBox& collision,bool blocksMovement = true) = 0;
    virtual bool move(EntityId id, const Position& base) = 0;
    virtual bool release(EntityId id) = 0;
    virtual bool hasMap() const = 0;
};
