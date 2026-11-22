#pragma once

#include <chrono>
#include <optional>

#include "dungeon/combat/dungeon_combat.h"
#include "dungeon/loading/dungeon_loader.h"
#include "dungeon/mining/dungeon_mining.h"
#include "dungeon/mining/mineral_generator.h"
#include "dungeon/rooms/dungeon_travel.h"

class PlayerSnapshot;
class PlayerManager;

class DungeonManager {
private:
    std::map<uint8_t, DungeonRoom> rooms;
    MineralGenerator generator;
    std::mt19937 placementRandom;
    DungeonMining mining;
    DungeonTravel travel;
    DungeonLoader loader;
    DungeonCombat combat;
    DungeonRoom* findRoom(uint8_t number);
    const DungeonRoom* findRoom(uint8_t number) const;

public:
    explicit DungeonManager(const DungeonConfig& config = {}, uint32_t seed = std::random_device{}());
    bool initialize(const std::map<uint8_t, DungeonRoomLayout>& layout, EntityIdGenerator& entityIds);
    std::optional<WorldLocation> spawnFor(const PlayerSnapshot& player) const;
    bool canWalk(uint8_t number, const CollisionBounds& bounds) const;
    std::optional<MineData> getMineData(uint8_t number, bool includeBroken = false) const;
    std::vector<PortalData> getPortals(uint8_t number) const;
    bool mineMineral(PlayerManager& players, uint16_t playerId, EntityId id, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now());
    std::vector<LootData> getGroundItems(uint8_t number) const;
    bool collectNearbyItems(PlayerManager& players, uint16_t playerId);
    bool useSelectedItem(PlayerManager& players, uint16_t playerId, Direction direction, std::chrono::steady_clock::time_point now, EntityIdGenerator& ids);
    void update(
        std::chrono::milliseconds elapsed, const std::vector<Combatant>& combatants, PlayerManager& players, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now);
    void onTravel(uint16_t playerId, uint8_t room, bool survived);
    const CombatProgressData& getCombatProgress() const;
    bool combatCompleted() const;
    std::optional<DungeonCombatData> getCombatData(uint8_t room) const;
    bool enter(PlayerManager& players, uint16_t playerId);
    bool advanceRoom(PlayerManager& players, uint16_t playerId);
    bool teleportToRoom(PlayerManager& players, uint16_t playerId, uint8_t number) const;
    bool spawnEnemies(
        const PlayerManager& players, uint16_t playerId, EnemyType type, uint16_t quantity, bool pairs, EntityIdGenerator& ids, const std::vector<Position>& occupied);
    uint16_t killEnemies(PlayerManager& players, uint16_t playerId, bool nearestOnly, EntityIdGenerator& ids);
    bool canExit(const PlayerSnapshot& player) const;
    void removeBrokenMinerals();
    std::optional<OpenChestData> openChestFor(const PlayerSnapshot& player) const;
    void depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command);
    void withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command);
    std::map<uint8_t, std::vector<FurnitureData>> storageFurniture() const;
    std::map<uint8_t, std::vector<LootData>> storageLoot() const;
    void saveInventories(std::map<uint32_t, std::map<uint8_t, Stack>>& inventories) const;
    void restoreStorage(const std::map<uint8_t, std::vector<FurnitureData>>& saved, const std::map<uint32_t, std::map<uint8_t, Stack>>& inventories,
        const std::map<uint8_t, std::vector<LootData>>& loot);
};
