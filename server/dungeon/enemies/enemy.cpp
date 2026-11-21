#include "enemy.h"

#include <algorithm>
#include <stdexcept>

Enemy::Enemy(uint32_t id, EnemyType type, const Position& pos, const EnemyDefinition& definition):
        definition(definition) {
    if (id == 0 || type > EnemyType::Skull || definition.health == 0) {
        throw std::invalid_argument("Invalid enemy");
    }
    data.id = id;
    data.type = type;
    data.pos = pos;
    data.dir = Direction::Down;
    data.health = definition.health;
    data.maxHealth = definition.health;
    setState(EnemyState::Idle);
}

bool Enemy::alive() const { return data.health > 0; }

void Enemy::setState(EnemyState state) {
    const bool flying = data.type == EnemyType::VampireMale || data.type == EnemyType::VampireFemale;
    if (!alive()) {
        data.state = EnemyState::Dead;
    } else if (flying) {
        data.state = EnemyState::Moving;
    } else {
        data.state = state;
    }
}

bool Enemy::hit(uint16_t amount, CombatTime now, const EnemyConfig& config) {
    if (!alive() || data.type == EnemyType::Skull || amount == 0) {
        return false;
    }
    data.health -= std::min(data.health, amount);
    ++data.action;
    if (alive()) {
        return false;
    }
    die(now, config);
    return true;
}

bool Enemy::kill(CombatTime now, const EnemyConfig& config) {
    if (!alive()) return false;
    data.health = 0;
    ++data.action;
    die(now, config);
    return true;
}

void Enemy::die(CombatTime now, const EnemyConfig& config) {
    setState(EnemyState::Dead);
    data.frenzied = false;
    movementRemainder = 0;
    waypoint.reset();
    targetPosition.reset();
    if (data.type == EnemyType::Skeleton) {
        reviveAt = now + CombatTime{config.skeletonReviveMs};
    } else if (data.type == EnemyType::Orc || data.type == EnemyType::Skull) {
        removeAt = now + CombatTime{2000};
    }
}

void Enemy::revive() {
    data.health = data.maxHealth;
    setState(EnemyState::Idle);
    data.frenzied = false;
    ++data.action;
    reviveAt.reset();
    removeAt.reset();
    waypoint.reset();
    targetPosition.reset();
    movementRemainder = 0;
    attackingUntil = CombatTime(0);
    targetId = 0;
}

uint16_t Enemy::damage() const {
    return static_cast<uint16_t>(definition.damage *(data.frenzied && data.type == EnemyType::VampireMale ? 2 : 1));
}

uint32_t Enemy::speed() const {
    return static_cast<uint32_t>(definition.speed) *(data.frenzied && data.type == EnemyType::VampireFemale ? 2 : 1);
}

CollisionBounds Enemy::bounds(const Position& pos) const { return CollisionBox{12, 8}.at(pos); }
