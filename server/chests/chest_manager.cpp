#include "chest_manager.h"

#include <limits>

#include "farm/spatial/farm_reservation.h"

bool ChestManager::place(EntityId id, const Position& position, FarmPlacement& placement, const std::map<uint8_t, Stack>& items) {
    if (id == 0 || placement.contains(id) || position.getX() < 0 || position.getY() <= 0 || position.getX() > std::numeric_limits<uint16_t>::max() ||
        position.getY() > std::numeric_limits<uint16_t>::max())
        return false;
    const Chest chest{id, position, items};
    FarmReservation reservation{placement, id, position, CollisionBox{entity_dimensions::DECORATION}};
    if (!reservation.isReserved() || !chests.emplace(id, chest).second)
        return false;
    reservation.commit();
    return true;
}

bool ChestManager::removeEmpty(EntityId id, FarmPlacement& placement) {
    const auto it = chests.find(id);
    if (it == chests.end() || !it->second.isEmpty())
        return false;
    placement.release(id);
    chests.erase(it);
    return true;
}

const Chest* ChestManager::find(EntityId id) const {
    const auto it = chests.find(id);
    return it == chests.end() ? nullptr : &it->second;
}

std::optional<ChestSnapshot> ChestManager::snapshot(EntityId id) const {
    const auto* chest = find(id);
    return chest ? std::optional<ChestSnapshot>{chest->snapshot()} : std::nullopt;
}

bool ChestManager::apply(EntityId id, PlayerManager& players, uint16_t playerId, const ChestAction& action) {
    const auto it = chests.find(id);
    return it != chests.end() && commands.apply(it->second, players, playerId, action);
}

const std::map<EntityId, Chest>& ChestManager::getChests() const { return chests; }
