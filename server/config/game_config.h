#pragma once

#include <cstdint>
#include <vector>
#include <optional>

#include "dungeon/dungeon_config.h"
#include "farm/farm_config.h"
#include "player/player_config.h"
#include "protocol_types.h"
#include "farm/loading/farm_layout.h"
#include "town/town_config.h"
#include "commerce/shop_config.h"
#include "community/domain/community_config.h"


struct DeathConfig {
    uint32_t goldDrop = 100;
};

struct GameConfig {
    DungeonConfig dungeon;
    PlayerConfig player;
    FarmConfig farm{};
    TownConfig town;
    DeathConfig death;
    RobinShopConfig robin;
    MarketConfig market;
    CommunityConfig community;
};

struct WorldData {
    GameConfig config;
    std::optional<FarmLayout> farm;
};
