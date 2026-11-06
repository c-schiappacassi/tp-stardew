#pragma once

#include <cstdint>
#include <map>
#include "dungeon/dungeon_config.h"
#include "dungeon/dungeon_entrance.h"
#include "dungeon/loading/dungeon_layout.h"
#include "player/player_config.h"
#include "farm/farm_config.h"
#include "farm/loading/farm_layout.h"

struct GameSetup {
    uint8_t worldId = 0;
    std::map<uint8_t, DungeonRoomLayout> layout;
    DungeonEntrance entrance;
    DungeonConfig dungeonConfig;
    PlayerConfig playerConfig;
    FarmLayout farm;
    FarmConfig farmConfig;
};

// Temporary playable layout until maps can be loaded from the editor.
GameSetup makeDefaultGameSetup(uint8_t worldId);
