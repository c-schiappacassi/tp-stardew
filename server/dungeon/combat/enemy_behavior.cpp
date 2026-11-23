#include "enemy_behavior.h"

#include <algorithm>
#include <limits>

#include "dungeon/dungeon_room.h"
#include "dungeon/enemies/enemy_navigation.h"
#include "player/player_manager.h"

#include "combat_context.h"
#include "combat_damage.h"
#include "projectile_combat.h"

namespace {
bool canReviveAt(const Enemy& enemy, const Position& position, const DungeonRoom& room, const std::vector<Combatant>& players) {
    const auto bounds = enemy.bounds(position);
    if (!room.canWalk(bounds)) {
        return false;
    }
    for (const auto& actor: players) {
        if (bounds.intersects(actor.player.getCollisionBounds(actor.player.getPosition()))) {
            return false;
        }
    }
    for (const auto& [id, other]: room.getCombat().enemies) {
        if (other.alive() && bounds.intersects(other.bounds(other.getPosition()))) {
            return false;
        }
    }
    return true;
}

std::optional<Position> findRevivePosition(const Enemy& enemy, const DungeonRoom& room, const std::vector<Combatant>& players) {
    if (canReviveAt(enemy, enemy.getPosition(), room, players)) {
        return enemy.getPosition();
    }
    std::optional<Position> nearest;
    double nearestDistance = std::numeric_limits<double>::max();
    for (const auto& position: room.getMap().walkableBasePositions()) {
        const double distance = enemy_navigation::squaredDistance(position, enemy.getPosition());
        if (distance < nearestDistance && canReviveAt(enemy, position, room, players)) {
            nearest = position;
            nearestDistance = distance;
        }
    }
    return nearest;
}

void tryRevive(Enemy& enemy, const DungeonRoom& room, const std::vector<Combatant>& players, CombatTime time) {
    if (!enemy.readyToRevive(time)) {
        return;
    }
    const auto position = findRevivePosition(enemy, room, players);
    if (!position) {
        return;
    }
    enemy.revive(*position, time);
}

const Combatant* findTarget(const Enemy& enemy, const std::vector<Combatant>& players, const PlayerManager& manager) {
    const Combatant* target = nullptr;
    double nearestDistance = std::numeric_limits<double>::max();
    for (const auto& actor: players) {
        if (!manager.snapshot(actor.id).canBeAttacked()) {
            continue;
        }
        const auto distance = enemy_navigation::squaredDistance(enemy.getPosition(), actor.player.getPosition());
        if (distance < nearestDistance || (distance == nearestDistance && target && actor.id < target->id)) {
            nearestDistance = distance;
            target = &actor;
        }
    }
    return target;
}

bool canReachTarget(const Enemy& enemy, const PlayerSnapshot& player, const DungeonRoom& room) {
    const auto range = enemy.getRange();
    const auto origin = enemy.bounds(enemy.getPosition()).center();
    const auto hitbox = player.getHitboxBounds();
    const Position target{static_cast<int>(std::clamp<int64_t>(origin.getX(), hitbox.getLeft(), hitbox.getRight())),
        static_cast<int>(std::clamp<int64_t>(origin.getY(), hitbox.getTop(), hitbox.getBottom()))};
    const auto distance = enemy_navigation::squaredDistance(origin, target);
    return distance <= static_cast<double>(range) * range && room.hasClearLine(origin, target);
}

void attack(CombatContext& context, Enemy& enemy, const Combatant& target, DungeonRoom& room) {
    if (!enemy.startAttack(context.time))
        return;

    if (enemy.getType() == EnemyType::Skull) {
        projectile_combat::launch(context, enemy, target.player, room);
    } else {
        combat_damage::apply(context, target.id, enemy.damage(), room);
    }
}

void moveTowardTarget(Enemy& enemy, const Combatant& target, const DungeonRoom& room, CombatTime elapsed) {
    if (enemy.getType() == EnemyType::Skull) {
        return;
    }
    const auto& targetPosition = target.player.getPosition();
    enemy.pursue(target.id, targetPosition, room.getMap());
    const int steps = enemy.movementSteps(elapsed);

    for (int step = 0; step < steps; ++step) {
        if (enemy.needsWaypoint(room.getMap(), targetPosition)) {
            enemy.setWaypoint(enemy_navigation::nextWaypoint(room, enemy.getPosition(), targetPosition));
        }
        if (!enemy.getWaypoint() || canReachTarget(enemy, target.player, room)) {
            return;
        }
        const auto direction = enemy_navigation::facing(enemy.getPosition(), *enemy.getWaypoint());
        const auto next = enemy_navigation::offsetPosition(enemy.getPosition(), direction, 1);
        if (!room.canWalk(enemy.bounds(next))) {
            enemy.clearWaypoint();
            return;
        }
        enemy.moveTo(next, direction);
    }
}

void updateEnemy(CombatContext& context, Enemy& enemy, DungeonRoom& room, const std::vector<Combatant>& players, CombatTime elapsed) {
    if (!enemy.alive()) {
        return;
    }
    const auto* target = findTarget(enemy, players, context.players);
    if (enemy.isAttacking(context.time)) {
        enemy.setState(EnemyState::Attacking);
        return;
    }
    enemy.setState(EnemyState::Idle);
    if (!target) {
        enemy.stopPursuit();
        return;
    }
    enemy.face(enemy_navigation::facing(enemy.getPosition(), target->player.getPosition()));
    if (canReachTarget(enemy, target->player, room)) {
        attack(context, enemy, *target, room);
    } else {
        moveTowardTarget(enemy, *target, room, elapsed);
    }
}
}  // namespace

void enemy_behavior::refreshVampirePairs(DungeonRoom& room) {
    auto& enemies = room.getCombat().enemies;
    for (auto& [id, enemy]: enemies) {
        const bool partnerDead = enemy.alive() && enemy.getPairId() != 0 && std::any_of(enemies.begin(), enemies.end(), [&](const auto& entry) {
            return entry.first != id && enemy.getPairId() == entry.second.getPairId() && !entry.second.alive();
        });
        enemy.setPairSurvivor(partnerDead);
    }
}

void enemy_behavior::update(CombatContext& context, DungeonRoom& room, const std::vector<Combatant>& players, CombatTime elapsed) {
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
        if (enemy.readyToRemove(context.time)) {
            it = enemies.erase(it);
        } else {
            ++it;
        }
    }
}
