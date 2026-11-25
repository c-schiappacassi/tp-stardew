#include "enemy.h"

#include "server_constants.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "world/zone_map.h"

#include "enemy_navigation.h"

Enemy::Enemy(uint32_t id, EnemyType type, const Position& pos, const EnemyDefinition& definition, uint32_t group, uint32_t pair):
    definition(definition), groupId(group), pairId(pair) {
    if (id == 0 || type > EnemyType::Skull || definition.health == 0) {
        throw std::invalid_argument(server_constants::INVALID_ENEMY);
    }
    data.id = id;
    data.type = type;
    data.pos = pos;
    data.dir = Direction::Down;
    data.health = definition.health;
    data.maxHealth = definition.health;
    setState(EnemyState::Idle);
}

const EnemySnapshot& Enemy::getPublicData() const { return data; }
uint32_t Enemy::getId() const { return data.id; }
EnemyType Enemy::getType() const { return data.type; }
const Position& Enemy::getPosition() const { return data.pos; }
uint32_t Enemy::getGroupId() const { return groupId; }
uint32_t Enemy::getPairId() const { return pairId; }
uint16_t Enemy::getRange() const { return definition.range; }

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
    if (!alive())
        return false;
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

void Enemy::revive(const Position& position, CombatTime now) {
    data.pos = position;
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
    nextAttack = now + CombatTime{definition.attackMs};
}

uint16_t Enemy::damage() const { return static_cast<uint16_t>(definition.damage * (data.frenzied && data.type == EnemyType::VampireMale ? 2 : 1)); }

uint32_t Enemy::speed() const { return static_cast<uint32_t>(definition.speed) * (data.frenzied && data.type == EnemyType::VampireFemale ? 2 : 1); }

CollisionBounds Enemy::bounds(const Position& pos) const { return CollisionBox{12, 8}.at(pos); }

bool Enemy::readyToRevive(CombatTime now) const { return reviveAt && now >= *reviveAt; }
bool Enemy::readyToRemove(CombatTime now) const { return removeAt && now >= *removeAt; }
bool Enemy::needsBloodRevival() const { return !alive() && pairId != 0 && !reviveAt; }

void Enemy::receiveBlood(CombatTime now, const EnemyConfig& config) {
    if (needsBloodRevival())
        reviveAt = now + CombatTime{config.vampireReviveMs};
}

void Enemy::setPairSurvivor(bool partnerDead) { data.frenzied = alive() && pairId != 0 && partnerDead; }

bool Enemy::startAttack(CombatTime now) {
    if (now < nextAttack)
        return false;
    nextAttack = now + CombatTime{definition.attackMs};
    setState(EnemyState::Attacking);
    attackingUntil = now + CombatTime{std::min<uint32_t>(360, definition.attackMs)};
    ++data.action;
    return true;
}

bool Enemy::isAttacking(CombatTime now) const { return now < attackingUntil; }
void Enemy::face(Direction direction) { data.dir = direction; }

void Enemy::moveTo(const Position& position, Direction direction) {
    data.pos = position;
    data.dir = direction;
    setState(EnemyState::Moving);
}

void Enemy::pursue(uint16_t playerId, const Position& position, const ZoneMap& map) {
    if (targetId != playerId || !targetPosition || !map.sameBaseCell(*targetPosition, position)) {
        targetId = playerId;
        waypoint.reset();
    }
    targetPosition = position;
}

void Enemy::stopPursuit() {
    waypoint.reset();
    targetPosition.reset();
    movementRemainder = 0;
}

int Enemy::movementSteps(CombatTime elapsed) {
    movementRemainder += speed() * static_cast<double>(elapsed.count()) / 1000.0;
    const int steps = static_cast<int>(movementRemainder);
    movementRemainder -= steps;
    return steps;
}

bool Enemy::needsWaypoint(const ZoneMap& map, const Position& target) const {
    return !waypoint || enemy_navigation::squaredDistance(data.pos, *waypoint) < 1 || map.sameBaseCell(data.pos, target);
}

const std::optional<Position>& Enemy::getWaypoint() const { return waypoint; }
void Enemy::setWaypoint(std::optional<Position> next) { waypoint = std::move(next); }
void Enemy::clearWaypoint() { waypoint.reset(); }
