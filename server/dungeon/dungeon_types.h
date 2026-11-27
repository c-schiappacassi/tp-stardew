#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include "chests/loading/chest_spawn.h"
#include "dtos.h"
#include "position.h"
#include "protocol_types.h"
#include "world/zone_map.h"

// Mining
struct MiningResult {
    bool acted = false;
    bool droppedSilver = false;
};

// Enemy Definitions & Combat Config
struct EnemyDefinition {
    uint16_t health = 40;
    uint16_t damage = 8;
    uint16_t speed = 32;
    uint16_t range = 20;
    uint32_t attackMs = 1000;
};

struct EnemyConfig {
    std::map<EnemyType, EnemyDefinition> definitions{{EnemyType::Skeleton, {40, 8, 32, 20, 1000}}, {EnemyType::Orc, {70, 12, 36, 20, 1200}},
        {EnemyType::VampireMale, {100, 18, 40, 22, 1000}}, {EnemyType::VampireFemale, {90, 12, 60, 22, 900}}, {EnemyType::Skull, {1, 10, 0, 160, 1500}}};
    uint32_t skeletonReviveMs = 7000;
    uint32_t vampireReviveMs = 3000;
    uint16_t bloodRadius = 80;
    uint16_t flameSpeed = 80;
    uint32_t flameLifetimeMs = 4000;
    uint32_t trapInactiveMs = 2000;
    uint32_t trapWarningMs = 700;
    uint32_t trapActiveMs = 1000;
    uint32_t trapHitMs = 800;
    uint16_t trapDamage = 12;
    uint16_t wallFlameRange = 64;
    uint16_t spawnSafetyRange = 64;
    uint8_t skeletonsPerRoom = 1;
    uint8_t spikesPerRoom = 1;
    uint8_t wallFlamesPerRoom = 1;
    uint8_t orcGroupEvery = 3;
    uint8_t vampirePairEvery = 4;
    uint8_t skullEvery = 5;
    uint8_t orcSilverChance = 20;
    uint8_t miningAmbushChance = 10;
    uint8_t goldChance = 75;
    uint16_t goldMin = 2;
    uint16_t goldMax = 10;
    uint16_t maxEnemiesPerRoom = 32;

    void validate() const;
};

// Entrance & Room Passages
struct DungeonEntrance {
    Zone zone = Zone::Farm;
    Position position{0, 0};
    Position returnPosition{0, 0};
    ZoneMap terrain;
};

struct DungeonRoomPassages {
    Position entry{0, 0};
    Position exit{0, 0};
    std::optional<Position> nextRoom;
    std::vector<Position> extraExits;
};

// Room Layout
struct DungeonRoomLayout {
    ZoneMap terrain;
    DungeonRoomPassages passages;
    std::vector<Position> reservedPositions;
    std::vector<TrapData> traps;
    std::vector<ChestSpawn> chests;
};

// Dungeon Config & Minerals
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
    std::vector<MineralDefinition> minerals{{ResourceType::CoalRock, 2, 60, 1, {ItemId::Coal, 1, Quality::None}, 100},
        {ResourceType::LapisRock, 4, 25, 1, {ItemId::Lapis, 1, Quality::None}, 85}, {ResourceType::SilverRock, 6, 10, 1, {ItemId::Silver, 1, Quality::None}, 100},
        {ResourceType::GoldRock, 8, 5, 5, {ItemId::Gold, 1, Quality::None}, 100}};
    uint16_t miningRange = 64;
    uint8_t mineralDensityPercent = 10;
    std::map<uint8_t, uint8_t> mineralDensityByRoom;
};
