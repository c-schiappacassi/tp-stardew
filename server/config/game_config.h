#pragma once

#include <cstdint>
#include <vector>
#include <optional>

#include "dungeon/dungeon_config.h"
#include "farm/farm_config.h"
#include "player/player_config.h"
#include "protocol_types.h"
#include "farm/farm_layout.h"
#include "town/town_config.h"


struct DeathConfig {
    uint32_t goldDrop = 100;
};

struct RobinProductConfig {
    ItemId item;
    uint32_t price;
    bool rotates;
};

struct RobinShopConfig {
    uint8_t dailyRotationCount = 7;
    std::vector<RobinProductConfig> products;
};

struct GameConfig {
    DungeonConfig dungeon;
    PlayerConfig player;
    FarmConfig farm;
    TownConfig town;
    DeathConfig death;
    RobinShopConfig robin;
};

struct WorldData {
    GameConfig config;
    std::optional<FarmLayout> farm;
};
