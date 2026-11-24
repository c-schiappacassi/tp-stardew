#include "dungeon_manager.h"

DungeonManager::DungeonManager(const DungeonConfig& config, uint32_t seed):
    generator(config, seed), mining(config.miningRange), loader(config, seed), combat(config.enemies, seed), interactions(rooms, mining, generator, combat) {}

bool DungeonManager::initialize(const std::map<uint8_t, DungeonRoomLayout>& layout, EntityIdGenerator& entityIds) {
    return loader.initialize(layout, rooms, generator, combat, entityIds);
}

std::optional<WorldLocation> DungeonManager::spawnFor(const PlayerSnapshot& player) const { return travel.spawnFor(player, rooms); }
bool DungeonManager::canWalk(uint8_t number, const CollisionBounds& bounds) const { return rooms.canWalk(number, bounds); }

std::optional<MineData> DungeonManager::getMineData(uint8_t number, bool includeBroken) const { return rooms.getMineData(number, combat.getVisit(), includeBroken); }

std::vector<PortalData> DungeonManager::getPortals(uint8_t number) const { return rooms.getPortals(number); }

std::optional<DungeonCombatData> DungeonManager::getCombatData(uint8_t number) const { return rooms.getCombatData(number, combat.getVisit()); }

bool DungeonManager::mineMineral(PlayerManager& players, uint16_t playerId, EntityId id, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) {
    return interactions.mineMineral(players, playerId, id, ids, now);
}

bool DungeonManager::useSelectedItem(PlayerManager& players, uint16_t playerId, Direction direction, std::chrono::steady_clock::time_point now, EntityIdGenerator& ids) {
    return interactions.useSelectedItem(players, playerId, direction, now, ids);
}

std::vector<LootData> DungeonManager::getGroundItems(uint8_t number) const { return rooms.getGroundItems(number); }
bool DungeonManager::collectNearbyItems(PlayerManager& players, uint16_t playerId) { return interactions.collectNearbyItems(players, playerId); }

void DungeonManager::update(
    std::chrono::milliseconds elapsed, const std::vector<Combatant>& combatants, PlayerManager& players, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) {
    combat.update(elapsed, rooms, combatants, players, ids, now);
}

void DungeonManager::onTravel(uint16_t playerId, uint8_t room, bool survived) { combat.onTravel(playerId, room, survived, rooms); }
CombatProgressData DungeonManager::getCombatProgress() const { return combat.getProgress(); }
bool DungeonManager::combatCompleted() const { return combat.completed(); }
bool DungeonManager::enter(PlayerManager& players, uint16_t playerId) { return travel.enter(players, playerId, rooms); }
bool DungeonManager::advanceRoom(PlayerManager& players, uint16_t playerId) { return travel.advanceRoom(players, playerId, rooms); }

bool DungeonManager::teleportToRoom(PlayerManager& players, uint16_t playerId, uint8_t number) const { return travel.teleportToRoom(players, playerId, number, rooms); }

bool DungeonManager::canExit(const PlayerSnapshot& player) const { return travel.canExit(player, rooms); }

bool DungeonManager::spawnEnemies(
    const PlayerManager& players, uint16_t playerId, EnemyType type, uint16_t quantity, bool pairs, EntityIdGenerator& ids, const std::vector<Position>& occupied) {
    return combat.spawnEnemies(players, playerId, rooms, type, quantity, pairs, ids, occupied);
}

uint16_t DungeonManager::killEnemies(PlayerManager& players, uint16_t playerId, bool nearestOnly, EntityIdGenerator& ids) {
    return combat.killEnemies(players, playerId, rooms, nearestOnly, ids);
}

void DungeonManager::removeBrokenMinerals() { rooms.removeBrokenMinerals(); }
std::optional<OpenChestData> DungeonManager::openChestFor(const PlayerSnapshot& player) const { return rooms.openChestFor(player); }

bool DungeonManager::depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command) { return interactions.depositInChest(players, playerId, command); }

bool DungeonManager::withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command) {
    return interactions.withdrawFromChest(players, playerId, command);
}

std::map<uint8_t, std::vector<FurnitureData>> DungeonManager::storageFurniture() const { return persistence.storageFurniture(rooms); }
std::map<uint8_t, std::vector<LootData>> DungeonManager::storageLoot() const { return persistence.storageLoot(rooms); }

void DungeonManager::saveInventories(std::map<uint32_t, std::map<uint8_t, Stack>>& inventories) const { persistence.saveInventories(rooms, inventories); }

void DungeonManager::restoreStorage(const std::map<uint8_t, std::vector<FurnitureData>>& saved, const std::map<uint32_t, std::map<uint8_t, Stack>>& inventories,
    const std::map<uint8_t, std::vector<LootData>>& loot) {
    persistence.restoreStorage(rooms, saved, inventories, loot);
}
