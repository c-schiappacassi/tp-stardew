#pragma once

#include <cstdint>
#include <map>
#include <optional>

#include "dungeon/contracts/dungeon_entrance.h"
#include "dungeon/contracts/dungeon_layout.h"
#include "farm/loading/farm_layout.h"
#include "town/town_config.h"

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
