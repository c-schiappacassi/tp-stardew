#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include "enemies/enemy_definition.h"

#include "dtos.h"

inline constexpr uint8_t DUNGEON_ROOM_COUNT = 10;

struct MineralDefinition {
    ResourceType type;
    uint16_t hitsToBreak;
    uint16_t spawnWeight;
    uint8_t firstRoom;
    Stack reward;
    uint8_t rewardChance;
};

struct DungeonConfig {
    EnemyConfig enemies;
    std::vector<MineralDefinition> minerals{
            {ResourceType::CoalRock, 2, 60, 1,{ItemId::Coal, 1, Quality::None}, 100},
            {ResourceType::LapisRock, 4, 25, 1,{ItemId::Lapis, 1, Quality::None}, 85},
            {ResourceType::SilverRock, 6, 10, 1,{ItemId::Silver, 1, Quality::None}, 100},
            {ResourceType::GoldRock, 8, 5, 5,{ItemId::Gold, 1, Quality::None}, 100}
        };
    uint16_t miningRange = 64;
    uint8_t mineralDensityPercent = 10;
    std::map<uint8_t, uint8_t> mineralDensityByRoom;
};
