#include "mineral.h"

#include "server_constants.h"

#include <algorithm>
#include <stdexcept>

#include "entities/collision_box.h"

Mineral::Mineral(EntityId id, const Position& pos, ResourceType type, uint16_t hitsToBreak):
    id(id), pos(pos), type(type), remainingDurability(static_cast<uint32_t>(hitsToBreak) * 2) {
    if (id == 0) {
        throw std::invalid_argument(server_constants::ENTITYID_MUST_BE_NONZERO);
    }

    if (hitsToBreak == 0) {
        throw std::invalid_argument(server_constants::THE_MINERAL_MUST_REQUIRE_AT_LEAST_ONE_HIT);
    }
}

EntityId Mineral::getId() const { return id; }

const Position& Mineral::getPosition() const { return pos; }

ResourceType Mineral::getType() const { return type; }

bool Mineral::isDepleted() const { return remainingDurability == 0; }

bool Mineral::hit(uint16_t power) {
    if (isDepleted()) {
        return false;
    }

    remainingDurability -= std::min<uint32_t>(remainingDurability, power);
    return isDepleted();
}

MineralState Mineral::getState() const {
    if (isDepleted()) {
        return MineralState::Broken;
    }

    return MineralState::Intact;
}

uint16_t Mineral::getHealth() const { return static_cast<uint16_t>(std::min<uint32_t>(remainingDurability, UINT16_MAX)); }

MineralData Mineral::getPublicData() const { return MineralData{id, pos, type, getState(), getHealth()}; }
CollisionBounds Mineral::getCollisionBounds() const { return CollisionBox{entity_dimensions::MINERAL}.at(pos); }

Position Mineral::getCollisionCenter() const { return getCollisionBounds().center(); }
