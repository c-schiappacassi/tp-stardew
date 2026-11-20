#pragma once

#include <chrono>
#include <optional>
#include "dungeon/combat/dungeon_combat.h"
#include "dungeon/loading/dungeon_loader.h"
#include "dungeon/mining/dungeon_mining.h"
#include "dungeon/mining/mineral_generator.h"
#include "dungeon/rooms/dungeon_travel.h"

class Player;

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
    explicit DungeonManager(const DungeonConfig& config = {},uint32_t seed = std::random_device{}());
    bool initialize(const std::map<uint8_t, DungeonRoomLayout>& layout,EntityIdGenerator& entityIds);
    bool spawnPlayer(Player& player) const;
    bool canWalk(uint8_t number, const CollisionBounds& bounds) const;
    std::optional<MineData> getMineData(uint8_t number, bool includeBroken = false) const;
    std::vector<PortalData> getPortals(uint8_t number) const;
    bool mineMineral(Player& player,EntityId id, EntityIdGenerator& ids,std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now());
    std::vector<LootData> getGroundItems(uint8_t number) const;
    bool collectNearbyItems(Player& player);
    bool useSelectedItem(Player& player,Direction direction,std::chrono::steady_clock::time_point now, EntityIdGenerator& ids,uint16_t playerId);
    void update(std::chrono::milliseconds elapsed, const std::vector<Combatant>& players,EntityIdGenerator& ids, std::chrono::steady_clock::time_point now);
    void onTravel(uint16_t playerId, uint8_t room, bool survived);
    const CombatProgressData& getCombatProgress() const;
    bool combatCompleted() const;
    std::optional<DungeonCombatData> getCombatData(uint8_t room) const;
    bool enter(Player& player);
    bool advanceRoom(Player& player);
    bool teleportToRoom(Player& player, uint8_t number) const;
    bool canExit(const Player& player) const;
    void removeBrokenMinerals();
};
