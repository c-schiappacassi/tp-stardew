#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include "position.h"

class DungeonManager;
class FarmManager;
class TownMap;
class InteriorRooms;
class EntityIdGenerator;
class WorldTravel;
struct FarmLayout;
struct TownConfig;
struct DungeonRoomLayout;
struct DungeonEntrance;

class WorldLoader {
private:
    DungeonManager& dungeon;
    FarmManager& farm;
    TownMap& town;
    InteriorRooms& interiors;
    EntityIdGenerator& entityIds;
    WorldTravel& travel;
    uint32_t farmSeed;
    std::optional<uint8_t> loadedWorld;

    bool validTownDoor(const TownMap& candidate, const Position& door, const Position& returnPosition, const std::vector<Position>& registered) const;

public:
    WorldLoader(DungeonManager& dungeon, FarmManager& farm, TownMap& town, InteriorRooms& interiors, EntityIdGenerator& ids, WorldTravel& travel, uint32_t seed);
    bool loadFarm(uint8_t worldId, const FarmLayout& layout);
    bool loadDungeon(uint8_t worldId, const std::map<uint8_t, DungeonRoomLayout>& layout);
    bool loadTown(const TownConfig& config);
    bool configureDungeonEntrance(const DungeonEntrance& entrance);
    bool acceptsWorld(uint8_t worldId) const;
};
