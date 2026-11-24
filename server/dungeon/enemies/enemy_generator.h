#pragma once

#include <random>
#include <vector>

#include "dungeon/contracts/enemy_definition.h"
#include "entities/entity_id_generator.h"

#include "position.h"

class DungeonRoom;

class EnemyGenerator {
    EnemyConfig config;
    std::mt19937 random;
    bool populateRoom(DungeonRoom& room, EntityIdGenerator& ids, const std::vector<Position>& reserved);
    bool spawn(DungeonRoom& room, const std::vector<EnemyType>& types, EntityIdGenerator& ids, const std::vector<Position>& reserved, uint32_t group, uint32_t pair);

public:
    EnemyGenerator(const EnemyConfig& config, uint32_t seed);
    bool populate(DungeonRoom& room, EntityIdGenerator& ids, const std::vector<Position>& reserved);
    bool spawnOrcs(DungeonRoom& room, EntityIdGenerator& ids, const std::vector<Position>& reserved, int minimum = 3);
    bool spawnEnemies(DungeonRoom& room, EnemyType type, uint16_t quantity, bool pairs, EntityIdGenerator& ids, const std::vector<Position>& reserved);
    bool roll(uint8_t percent);
    uint16_t gold();
};
