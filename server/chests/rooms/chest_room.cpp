#include "chest_room.h"

#include <stdexcept>
#include <utility>

#include "chests/interactions/chest_interactions.h"
#include "player/player_manager.h"
#include "world/zone_map.h"

bool ChestRoom::initialize(
    const ZoneMap& terrain, const std::vector<Position>& reserved, const std::vector<CollisionBounds>& obstacles, const std::vector<ChestSpawn>& initial, EntityIdGenerator& ids) {
    ChestRoom prepared;
    auto preparedIds = ids;
    if (!prepared.space.setMap(terrain) || !prepared.space.setReservedPositions(reserved))
        return false;
    prepared.fixedObstacles = obstacles;
    prepared.space.setObstacles(obstacles);
    for (const auto& spawn: initial) {
        try {
            if (preparedIds.getNextId() == 0 || !prepared.chests.place(preparedIds.getNextId(), spawn.base, prepared.space, spawn.items))
                return false;
        } catch (const std::invalid_argument&) {
            return false;
        }
        preparedIds.generate();
    }
    *this = std::move(prepared);
    ids = preparedIds;
    return true;
}

bool ChestRoom::canWalk(const CollisionBounds& bounds) const { return space.canWalk(bounds); }
std::vector<FurnitureData> ChestRoom::furniture() const {
    std::vector<FurnitureData> result;
    for (const auto& [id, chest]: chests.getChests()) result.push_back(chest.getPublicData());
    return result;
}

std::optional<bool> ChestRoom::interact(PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now,
    const std::vector<CollisionBounds>& obstacles) {
    auto occupied = fixedObstacles;
    occupied.insert(occupied.end(), obstacles.begin(), obstacles.end());
    space.setObstacles(occupied);
    return ChestInteractions{}.interact(players, playerId, chests, space, space.getMap(), groundItems, direction, ids, now);
}

bool ChestRoom::deposit(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command) {
    return ChestInteractions{}.deposit(players, playerId, chests, space.getMap(), command);
}

bool ChestRoom::withdraw(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command) {
    return ChestInteractions{}.withdraw(players, playerId, chests, space.getMap(), command);
}

std::optional<OpenChestData> ChestRoom::contentsFor(const PlayerSnapshot& player) const { return ChestInteractions{}.contentsFor(player, chests, space.getMap()); }

const std::map<EntityId, Chest>& ChestRoom::getChests() const { return chests.getChests(); }
GroundItems& ChestRoom::getGroundItems() { return groundItems; }
const GroundItems& ChestRoom::getGroundItems() const { return groundItems; }

bool ChestRoom::collectNearbyItems(PlayerManager& players, uint16_t playerId) {
    const auto player = players.snapshot(playerId);
    return groundItems.collect(players, playerId, [&](const auto& position) { return space.getMap().hasClearPath(player.getPosition(), position); });
}

void ChestRoom::restore(const std::vector<FurnitureData>& furniture, const std::map<uint32_t, std::map<uint8_t, Stack>>& inventories, const std::vector<LootData>& loot) {
    auto prepared = *this;
    prepared.chests = ChestManager{};
    prepared.space.clearOccupants();
    prepared.space.setObstacles(fixedObstacles);
    for (const auto& item: furniture) {
        if (item.type != FurnitureId::Chest)
            continue;
        const auto contents = inventories.find(item.id);
        if (contents == inventories.end() || !prepared.chests.place(item.id, item.pos, prepared.space, contents->second))
            throw std::runtime_error("Invalid saved chest furniture or contents");
    }
    prepared.groundItems.restoreItems(loot);
    *this = std::move(prepared);
}
