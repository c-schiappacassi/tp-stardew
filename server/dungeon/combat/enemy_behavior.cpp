#include "enemy_behavior.h"

#include <algorithm>
#include <limits>

#include "dungeon/enemies/enemy_navigation.h"
#include "dungeon/rooms/dungeon_room.h"
#include "player/player.h"

#include "combat_context.h"
#include "combat_damage.h"
#include "projectile_combat.h"

namespace {
bool canReviveAt(const Enemy& enemy, const Position& position, const DungeonRoom& room,const std::vector<Combatant>& players) {
    const auto bounds = enemy.bounds(position);
    if (!room.canWalk(bounds)) {
        return false;
    }
    for (const auto& actor: players) {
        if (bounds.intersects(actor.player->getCollisionBounds(actor.player->getPosition()))) {
            return false;
        }
    }
    for (const auto& [id, other]: room.getCombat().enemies) {
        if (other.alive() && bounds.intersects(other.bounds(other.data.pos))) {
            return false;
        }
    }
    return true;
}

std::optional<Position> findRevivePosition(const Enemy& enemy, const DungeonRoom& room,const std::vector<Combatant>& players) {
    if (canReviveAt(enemy, enemy.data.pos, room, players)) {
        return enemy.data.pos;
    }
    std::optional<Position> nearest;
    double nearestDistance = std::numeric_limits<double>::max();
    for (const auto& position: room.getMap().walkableBasePositions()) {
        const double distance = enemy_navigation::squaredDistance(position, enemy.data.pos);
        if (distance < nearestDistance && canReviveAt(enemy, position, room, players)) {
            nearest = position;
            nearestDistance = distance;
        }
    }
    return nearest;
}

void tryRevive(Enemy& enemy, const DungeonRoom& room, const std::vector<Combatant>& players,CombatTime time) {
    if (!enemy.reviveAt || time < *enemy.reviveAt) {
        return;
    }
    const auto position = findRevivePosition(enemy, room, players);
    if (!position) {
        return;
    }
    enemy.data.pos = *position;
    enemy.revive();
    enemy.nextAttack = time + CombatTime{enemy.definition.attackMs};
}

const Combatant* findTarget(const Enemy& enemy, const std::vector<Combatant>& players) {
    const Combatant* target = nullptr;
    double nearestDistance = std::numeric_limits<double>::max();
    for (const auto& actor: players) {
        if (!actor.player->canBeAttacked()) {
            continue;
        }
        const auto distance =
                enemy_navigation::squaredDistance(enemy.data.pos, actor.player->getPosition());
        if (distance < nearestDistance ||
            (distance == nearestDistance && target && actor.id < target->id)) {
            nearestDistance = distance;
            target = &actor;
        }
    }
    return target;
}

bool canReachTarget(const Enemy& enemy, const Player& player, const DungeonRoom& room) {
    const auto range = enemy.definition.range;
    const auto distance = enemy_navigation::squaredDistance(enemy.data.pos, player.getPosition());
    return distance <= static_cast<double>(range) * range &&
           room.hasClearLine(enemy.bounds(enemy.data.pos).center(), player.getCollisionCenter());
}

void attack(CombatContext& context, Enemy& enemy, const Combatant& target, DungeonRoom& room) {
    if (context.time < enemy.nextAttack) {
        return;
    }
    enemy.nextAttack = context.time + CombatTime{enemy.definition.attackMs};
    enemy.setState(EnemyState::Attacking);
    enemy.attackingUntil =
            context.time + CombatTime{std::min<uint32_t>(360, enemy.definition.attackMs)};
    ++enemy.data.action;

    if (enemy.data.type == EnemyType::Skull) {
        projectile_combat::launch(context, enemy, *target.player, room);
    } else {
        combat_damage::apply(context, *target.player, target.id, enemy.damage(), room);
    }
}

void moveTowardTarget(Enemy& enemy, const Combatant& target, const DungeonRoom& room,CombatTime elapsed) {
    if (enemy.data.type == EnemyType::Skull) {
        return;
    }
    const auto& targetPosition = target.player->getPosition();
    if (enemy.targetId != target.id || !enemy.targetPosition ||
        !room.getMap().sameBaseCell(*enemy.targetPosition, targetPosition)) {
        enemy.targetId = target.id;
        enemy.waypoint.reset();
    }
    enemy.targetPosition = targetPosition;
    enemy.movementRemainder += enemy.speed() * static_cast<double>(elapsed.count()) / 1000.0;
    const int steps = static_cast<int>(enemy.movementRemainder);
    enemy.movementRemainder -= steps;

    for (int step = 0; step < steps; ++step) {
        if (!enemy.waypoint ||
            enemy_navigation::squaredDistance(enemy.data.pos, *enemy.waypoint) < 1 ||
            room.getMap().sameBaseCell(enemy.data.pos, targetPosition)) {
            enemy.waypoint = enemy_navigation::nextWaypoint(room, enemy.data.pos, targetPosition);
        }
        if (!enemy.waypoint || canReachTarget(enemy, *target.player, room)) {
            return;
        }
        const auto direction = enemy_navigation::facing(enemy.data.pos, *enemy.waypoint);
        const auto next = enemy_navigation::offsetPosition(enemy.data.pos, direction, 1);
        if (!room.canWalk(enemy.bounds(next))) {
            enemy.waypoint.reset();
            return;
        }
        enemy.data.pos = next;
        enemy.data.dir = direction;
        enemy.setState(EnemyState::Moving);
    }
}

void updateEnemy(CombatContext& context, Enemy& enemy, DungeonRoom& room,const std::vector<Combatant>& players, CombatTime elapsed) {
    if (!enemy.alive()) {
        return;
    }
    const auto* target = findTarget(enemy, players);
    if (context.time < enemy.attackingUntil) {
        enemy.setState(EnemyState::Attacking);
        return;
    }
    enemy.setState(EnemyState::Idle);
    if (!target) {
        enemy.waypoint.reset();
        enemy.targetPosition.reset();
        enemy.movementRemainder = 0;
        return;
    }
    enemy.data.dir = enemy_navigation::facing(enemy.data.pos, target->player->getPosition());
    if (canReachTarget(enemy, *target->player, room)) {
        attack(context, enemy, *target, room);
    } else {
        moveTowardTarget(enemy, *target, room, elapsed);
    }
}
}

void enemy_behavior::refreshVampirePairs(DungeonRoom& room) {
    auto& enemies = room.getCombat().enemies;
    for (auto& [id, enemy]: enemies) {
        enemy.data.frenzied = false;
        if (!enemy.alive() || enemy.pairId == 0) {
            continue;
        }
        for (const auto& [otherId, other]: enemies) {
            if (otherId != id && enemy.pairId == other.pairId && !other.alive()) {
                enemy.data.frenzied = true;
                break;
            }
        }
    }
}

void enemy_behavior::update(CombatContext& context, DungeonRoom& room,const std::vector<Combatant>& players, CombatTime elapsed) {
    auto& enemies = room.getCombat().enemies;
    for (auto& [id, enemy]: enemies) {
        if (!enemy.alive()) {
            tryRevive(enemy, room, players, context.time);
        }
    }
    refreshVampirePairs(room);
    for (auto& [id, enemy]: enemies) {
        updateEnemy(context, enemy, room, players, elapsed);
    }
    for (auto it = enemies.begin(); it != enemies.end();) {
        const auto& enemy = it->second;
        if (enemy.removeAt && context.time >= *enemy.removeAt) {
            it = enemies.erase(it);
        } else {
            ++it;
        }
    }
}
