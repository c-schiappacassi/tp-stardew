#include "mineral.h"
#include "entities/collision_box.h"

#include <stdexcept>

Mineral::Mineral(EntityId id,const Position& pos,ResourceType type,uint16_t hitsToBreak): id(id),pos(pos),type(type),remainingHits(hitsToBreak) {
    if (id == 0) {
        throw std::invalid_argument("EntityId must be nonzero");
    }

    if (hitsToBreak == 0) {
        throw std::invalid_argument("The mineral must require at least one hit");
    }
}

EntityId Mineral::getId() const {
    return id;
}

const Position& Mineral::getPosition() const {
    return pos;
}

ResourceType Mineral::getType() const {
    return type;
}

bool Mineral::isDepleted() const {
    return remainingHits == 0;
}

bool Mineral::hit() {
    if (isDepleted()) {
        return false;
    }

    --remainingHits;
    return remainingHits == 0;
}

MineralState Mineral::getState() const {
    if (isDepleted()) {
        return MineralState::Broken;
    }

    return MineralState::Intact;
}

MineralData Mineral::getPublicData() const {
    return MineralData{id, pos, type, getState()};
}
CollisionBounds Mineral::getCollisionBounds() const {
    return CollisionBox{entity_dimensions::MINERAL}.at(pos);
}

Position Mineral::getCollisionCenter() const {
    return getCollisionBounds().center();
}
