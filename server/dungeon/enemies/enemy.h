#pragma once

#include <chrono>
#include <optional>

#include "entities/collision_box.h"

#include "dtos.h"
#include "enemy_definition.h"

using CombatTime = std::chrono::milliseconds;

class Enemy {
public:
    EnemySnapshot data;
    EnemyDefinition definition;
    uint32_t groupId = 0;
    uint32_t pairId = 0;
    CombatTime nextAttack{0};
    CombatTime attackingUntil{0};
    std::optional<CombatTime> reviveAt;
    std::optional<CombatTime> removeAt;
    double movementRemainder = 0;
    std::optional<Position> waypoint;
    std::optional<Position> targetPosition;
    uint16_t targetId = 0;

    Enemy(uint32_t id, EnemyType type, const Position& pos, const EnemyDefinition& definition);
    bool alive() const;
    void setState(EnemyState state);
    bool hit(uint16_t damage, CombatTime now, const EnemyConfig& config);
    void revive();
    uint16_t damage() const;
    uint32_t speed() const;
    CollisionBounds bounds(const Position& pos) const;
};
