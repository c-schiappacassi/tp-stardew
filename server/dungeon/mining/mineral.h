#pragma once
#include <cstdint>
#include "entities/entity_id.h"
#include "position.h"
#include "protocol_types.h"
#include "dtos.h"

class Mineral {
private:
    EntityId id;
    Position pos;
    ResourceType type;
    uint16_t remainingHits;

public:
    Mineral(EntityId id,const Position& pos,ResourceType type,uint16_t hitsToBreak);

    EntityId getId() const;
    const Position& getPosition() const;
    ResourceType getType() const;
    bool isDepleted() const;
    bool hit();
    MineralState getState() const;
    MineralData getPublicData() const;
};