#pragma once

#include <cstdint>
#include <map>
#include <optional>

#include "dungeon/dungeon_types.h"
#include "farm/farm_types.h"
#include "town/town_config.h"
#include "world/world_types.h"

struct WorldSetup {
    uint8_t worldId = 0;
    std::map<uint8_t, DungeonRoomLayout> layout;
    std::optional<DungeonEntrance> entrance;
    FarmLayout farm;
    TownConfig townConfig;
};

struct PreparedWorld {
    WorldSetup setup;
    ConstMapData visuals;
};
