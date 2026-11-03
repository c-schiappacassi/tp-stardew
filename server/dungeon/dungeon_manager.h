#pragma once

#include <chrono>
#include <optional>
#include "dungeon/loading/dungeon_loader.h"
#include "dungeon/mining/dungeon_mining.h"
#include "dungeon/rooms/dungeon_travel.h"
#include "dungeon/mining/mineral_generator.h"

class Player;

class DungeonManager {
private:
    std::map<uint8_t, DungeonRoom> rooms;
    MineralGenerator generator;
    std::mt19937 placementRandom;
    DungeonMining mining;
    DungeonTravel travel;
    DungeonLoader loader;
    DungeonRoom* findRoom(uint8_t number);
    const DungeonRoom* findRoom(uint8_t number) const;

public:
    explicit DungeonManager(const DungeonConfig& config = {},uint32_t seed = std::random_device{}());
    bool initialize(const std::map<uint8_t, DungeonRoomLayout>& layout,EntityIdGenerator& entityIds);
    bool canWalk(uint8_t number, const Position& pos) const;
    bool canWalk(uint8_t number, const CollisionBounds& bounds) const;
    std::optional<MineData> getMineData(uint8_t number, bool includeBroken = false) const;
    bool mineMineral(Player& player,EntityId id,std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now());
    bool useSelectedItem(Player& player,Direction direction,std::chrono::steady_clock::time_point now);
    bool enter(Player& player);
    bool advanceRoom(Player& player);
    bool canExit(const Player& player) const;
    void removeBrokenMinerals();
};
