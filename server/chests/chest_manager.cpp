#include "chest_manager.h"

#include <limits>

#include "entities/placement/placement_reservation.h"

#include "interactions/chest_interactions.h"

bool ChestManager::place(EntityId id, const Position& position, Placement& placement, const std::map<uint8_t, Stack>& items) {
    if (id == 0 || placement.contains(id) || position.getX() < 0 || position.getY() <= 0 || position.getX() > std::numeric_limits<uint16_t>::max() ||
        position.getY() > std::numeric_limits<uint16_t>::max())
        return false;
    const Chest chest{id, position, items};
    PlacementReservation reservation{placement, id, position, CollisionBox{entity_dimensions::DECORATION}};
    if (!reservation.isReserved() || !chests.emplace(id, chest).second)
        return false;
    reservation.commit();
    return true;
}

bool ChestManager::removeEmpty(EntityId id, Placement& placement) {
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

std::vector<FurnitureData> ChestManager::furnitureData() const {
    std::vector<FurnitureData> result;
    result.reserve(chests.size());
    for (const auto& [id, chest]: chests)
        result.push_back(chest.getPublicData());
    return result;
}

std::map<uint32_t, std::map<uint8_t, Stack>> ChestManager::inventories() const {
    std::map<uint32_t, std::map<uint8_t, Stack>> result;
    for (const auto& [id, chest]: chests)
        result.emplace(id, chest.getInventory().getItems());
    return result;
}

std::optional<bool> ChestManager::interact(PlayerManager& players, uint16_t playerId, Placement& placement, const ZoneMap& terrain, GroundItems& groundItems,
    Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) {
    return ChestInteractions{}.interact(players, playerId, *this, placement, terrain, groundItems, direction, ids, now);
}

std::optional<OpenChestData> ChestManager::contentsFor(const PlayerSnapshot& player, const ZoneMap& terrain) const {
    return ChestInteractions{}.contentsFor(player, *this, terrain);
}

bool ChestManager::deposit(PlayerManager& players, uint16_t playerId, const ZoneMap& terrain, const ItemTransferDTO& command) {
    return ChestInteractions{}.deposit(players, playerId, *this, terrain, command);
}

bool ChestManager::withdraw(PlayerManager& players, uint16_t playerId, const ZoneMap& terrain, const SelectItemDTO& command) {
    return ChestInteractions{}.withdraw(players, playerId, *this, terrain, command);
}
