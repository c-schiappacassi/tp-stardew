#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include "dungeon/dungeon_config.h"
#include "dungeon/dungeon_entrance.h"
#include "dungeon/loading/dungeon_layout.h"
#include "player/player_config.h"
#include "farm/farm_config.h"
#include "farm/loading/farm_layout.h"
#include "town/town_config.h"

struct GameSetup {
    uint8_t worldId = 0;
    std::map<uint8_t, DungeonRoomLayout> layout;
    std::optional<DungeonEntrance> entrance;
    DungeonConfig dungeonConfig;
    PlayerConfig playerConfig;
    FarmLayout farm;
    FarmConfig farmConfig;
    TownConfig townConfig;
};

GameSetup makeDefaultGameSetup(uint8_t worldId);

GameSetup makeFarmGameSetup(uint8_t worldId, const FarmLayout& farm);
