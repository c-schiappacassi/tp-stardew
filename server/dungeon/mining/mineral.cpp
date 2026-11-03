#include "mineral.h"

#include <stdexcept>

Mineral::Mineral(EntityId id,const Position& pos,ResourceType type,uint16_t hitsToBreak): id(id),pos(pos),type(type),remainingHits(hitsToBreak) {
    if (id == 0) {
        throw std::invalid_argument("El EntityId debe ser distinto de cero");
    }

    if (hitsToBreak == 0) {
        throw std::invalid_argument("El mineral debe requerir al menos un golpe");
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