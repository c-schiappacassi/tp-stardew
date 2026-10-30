#pragma once

#include <cstdint>
#include <vector>
#include "protocol_types.h"

inline constexpr uint8_t DUNGEON_ROOM_COUNT = 10;

struct MineralDefinition {
    ResourceType type;
    uint16_t hitsToBreak;
    uint16_t spawnWeight;
    uint8_t firstRoom;
};

struct DungeonConfig {
    std::vector<MineralDefinition> minerals{
        {ResourceType::CoalRock, 2, 60, 1},
        {ResourceType::LapisRock, 4, 25, 1},
        {ResourceType::SilverRock, 6, 10, 1},
        {ResourceType::GoldRock, 8, 5, 5}
    };
};