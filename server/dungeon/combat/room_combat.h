#pragma once

#include <map>
#include <set>

#include "dungeon/enemies/enemy.h"

struct Projectile {
    ProjectileData data;
    double x = 0;
    double y = 0;
    double velocityX = 0;
    double velocityY = 0;
    uint16_t damage = 0;
    CombatTime expires{0};
    CombatTime createdAt{0};
};

struct Trap {
    TrapData data;
    CombatTime offset{0};
    std::map<uint16_t, CombatTime> nextHits;
};

struct RoomCombat {
    std::map<uint32_t, Enemy> enemies;
    std::map<uint32_t, Projectile> projectiles;
    std::map<uint32_t, Trap> traps;
    uint16_t pendingAmbushes = 0;
};
