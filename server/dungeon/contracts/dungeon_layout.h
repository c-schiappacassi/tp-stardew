#pragma once

#include <vector>

#include "chests/loading/chest_spawn.h"
#include "dungeon/contracts/dungeon_room_passages.h"
#include "world/zone_map.h"

#include "dtos.h"

struct DungeonRoomLayout {
    ZoneMap terrain;
    DungeonRoomPassages passages;
    std::vector<Position> reservedPositions;
    std::vector<TrapData> traps;
    std::vector<ChestSpawn> chests;
};
