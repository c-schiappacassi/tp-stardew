#pragma once

#include <map>
#include <random>
#include "dungeon_layout.h"
#include "dungeon/dungeon_config.h"
#include "dungeon/mining/mineral_placement.h"
#include "dungeon/rooms/dungeon_room.h"

class MineralGenerator;
class EntityIdGenerator;

class DungeonLoader {
private:
    uint8_t densityPercent;
    std::map<uint8_t, uint8_t> densitiesByRoom;
    MineralPlacement placement;

public:
    explicit DungeonLoader(const DungeonConfig& config = {});

    bool load(const std::map<uint8_t, DungeonRoomLayout>& layout,std::map<uint8_t, DungeonRoom>& rooms,MineralGenerator& generator,EntityIdGenerator& entityIds,std::mt19937& random) const;
};