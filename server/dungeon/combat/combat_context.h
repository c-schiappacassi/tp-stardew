#pragma once

#include "dungeon/enemies/enemy_generator.h"

#include "combat_progress.h"

class PlayerManager;

struct CombatContext {
    PlayerManager& players;
    const EnemyConfig& config;
    EnemyGenerator& generator;
    CombatProgress& progress;
    EntityIdGenerator& entityIds;
    CombatTime time;
    std::chrono::steady_clock::time_point actionTime;
};
