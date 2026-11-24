#pragma once

#include <map>
#include <random>

#include "dungeon/contracts/dungeon_layout.h"
#include "dungeon/dungeon_config.h"
#include "dungeon/mining/mineral_placement.h"

class MineralGenerator;
class EntityIdGenerator;
class DungeonRooms;
class DungeonCombat;

class DungeonLoader {
private:
    uint8_t densityPercent;
    std::map<uint8_t, uint8_t> densitiesByRoom;
    MineralPlacement placement;
    std::mt19937 random;

    bool load(const std::map<uint8_t, DungeonRoomLayout>& layout, DungeonRooms& rooms, MineralGenerator& generator, EntityIdGenerator& ids, std::mt19937& placementRandom) const;

public:
    DungeonLoader(const DungeonConfig& config, uint32_t seed);
    bool initialize(const std::map<uint8_t, DungeonRoomLayout>& layout, DungeonRooms& rooms, MineralGenerator& generator, DungeonCombat& combat, EntityIdGenerator& ids);
};
