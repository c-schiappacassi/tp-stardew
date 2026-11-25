#pragma once
#include <cstdint>

#include "entities/collision_bounds.h"
#include "entities/entity_id.h"

#include "dtos.h"
#include "position.h"
#include "protocol_types.h"

class Mineral {
private:
    EntityId id;
    Position pos;
    ResourceType type;
    uint32_t remainingDurability;

public:
    Mineral(EntityId id, const Position& pos, ResourceType type, uint16_t hitsToBreak);

    EntityId getId() const;
    const Position& getPosition() const;
    ResourceType getType() const;
    CollisionBounds getCollisionBounds() const;
    Position getCollisionCenter() const;
    bool isDepleted() const;
    bool hit(uint16_t power = 2);
    MineralState getState() const;
    uint16_t getHealth() const;
    MineralData getPublicData() const;
};
