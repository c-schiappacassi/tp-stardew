#pragma once

#include <chrono>
#include <optional>

#include "dungeon/contracts/enemy_definition.h"
#include "entities/collision_box.h"

#include "dtos.h"

class ZoneMap;

using CombatTime = std::chrono::milliseconds;

class Enemy {
private:
    EnemySnapshot data;
    EnemyDefinition definition;
    uint32_t groupId;
    uint32_t pairId;
    CombatTime nextAttack{0};
    CombatTime attackingUntil{0};
    std::optional<CombatTime> reviveAt;
    std::optional<CombatTime> removeAt;
    double movementRemainder = 0;
    std::optional<Position> waypoint;
    std::optional<Position> targetPosition;
    uint16_t targetId = 0;

    void die(CombatTime now, const EnemyConfig& config);

public:
    Enemy(uint32_t id, EnemyType type, const Position& pos, const EnemyDefinition& definition, uint32_t group = 0, uint32_t pair = 0);
    const EnemySnapshot& getPublicData() const;
    uint32_t getId() const;
    EnemyType getType() const;
    const Position& getPosition() const;
    uint32_t getGroupId() const;
    uint32_t getPairId() const;
    uint16_t getRange() const;
    bool alive() const;
    void setState(EnemyState state);
    bool hit(uint16_t damage, CombatTime now, const EnemyConfig& config);
    bool kill(CombatTime now, const EnemyConfig& config);
    bool readyToRevive(CombatTime now) const;
    bool readyToRemove(CombatTime now) const;
    bool needsBloodRevival() const;
    void receiveBlood(CombatTime now, const EnemyConfig& config);
    void revive(const Position& position, CombatTime now);
    void setPairSurvivor(bool partnerDead);
    bool startAttack(CombatTime now);
    bool isAttacking(CombatTime now) const;
    void face(Direction direction);
    void moveTo(const Position& position, Direction direction);
    void pursue(uint16_t playerId, const Position& position, const ZoneMap& map);
    void stopPursuit();
    int movementSteps(CombatTime elapsed);
    bool needsWaypoint(const ZoneMap& map, const Position& target) const;
    const std::optional<Position>& getWaypoint() const;
    void setWaypoint(std::optional<Position> next);
    void clearWaypoint();
    uint16_t damage() const;
    uint32_t speed() const;
    CollisionBounds bounds(const Position& pos) const;
};
