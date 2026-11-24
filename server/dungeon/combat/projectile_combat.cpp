#include "projectile_combat.h"

#include <algorithm>
#include <cmath>

#include "dungeon/dungeon_room.h"
#include "dungeon/enemies/enemy_navigation.h"
#include "player/player_manager.h"

#include "combat_context.h"
#include "combat_damage.h"

namespace {
bool hitPlayer(CombatContext& context, const Projectile& projectile, const std::vector<Combatant>& players, DungeonRoom& room) {
    const auto bounds = CollisionBox{4, 4}.at(projectile.data.pos);
    for (const auto& actor: players) {
        const auto player = context.players.snapshot(actor.id);
        if (player.canBeAttacked() && bounds.intersects(player.getHitboxBounds())) {
            combat_damage::apply(context, actor.id, projectile.damage, room);
            return true;
        }
    }
    return false;
}

bool advanceProjectile(CombatContext& context, Projectile& projectile, DungeonRoom& room, const std::vector<Combatant>& players, CombatTime elapsed) {
    if (context.time >= projectile.expires) {
        return false;
    }
    const auto age = std::max(CombatTime(0), context.time - projectile.createdAt);
    const auto movementTime = std::min(elapsed, age);
    const double dx = projectile.velocityX * static_cast<double>(movementTime.count()) / 1000.0;
    const double dy = projectile.velocityY * static_cast<double>(movementTime.count()) / 1000.0;
    const auto distance = std::max(std::abs(dx), std::abs(dy));
    const int steps = std::max(1, static_cast<int>(std::ceil(distance)));

    for (int step = 0; step < steps; ++step) {
        projectile.x += dx / steps;
        projectile.y += dy / steps;
        projectile.data.pos = Position{static_cast<int>(std::round(projectile.x)), static_cast<int>(std::round(projectile.y))};
        if (!room.canWalk(CollisionBox{4, 4}.at(projectile.data.pos)) || hitPlayer(context, projectile, players, room)) {
            return false;
        }
    }
    return true;
}
}  // namespace

void projectile_combat::launch(CombatContext& context, const Enemy& enemy, const PlayerSnapshot& target, DungeonRoom& room) {
    auto& projectiles = room.getCombat().projectiles;
    if (projectiles.size() >= 128 || context.entityIds.getNextId() == 0) {
        return;
    }
    const bool green = context.generator.roll(50);
    const auto origin = enemy.bounds(enemy.getPosition()).center();
    const auto destination = target.getHitboxCenter();
    const double dx = destination.getX() - origin.getX();
    const double dy = destination.getY() - origin.getY();
    const double length = std::max(1.0, std::hypot(dx, dy));
    double speed = context.config.flameSpeed;
    uint16_t damage = enemy.damage();
    FlameType type = FlameType::Blue;
    if (green) {
        speed /= 2.0;
        damage = static_cast<uint16_t>(damage * 2);
        type = FlameType::Green;
    }
    const auto id = context.entityIds.generate();
    projectiles.emplace(id,
        Projectile{ProjectileData{id, origin, type}, static_cast<double>(origin.getX()), static_cast<double>(origin.getY()), speed * dx / length, speed * dy / length, damage,
            context.time + CombatTime{context.config.flameLifetimeMs}, context.time});
}

void projectile_combat::launch(CombatContext& context, const TrapData& trap, DungeonRoom& room) {
    auto& projectiles = room.getCombat().projectiles;
    if (trap.type != TrapType::WallFlame || trap.range == 0 || trap.dir == Direction::None || trap.dir > Direction::Right || projectiles.size() >= 128 ||
        context.entityIds.getNextId() == 0) {
        return;
    }
    const auto origin = CollisionBox{16, 8}.at(trap.pos).center();
    const auto step = enemy_navigation::offsetPosition(origin, trap.dir, 1);
    const double speed = context.config.flameSpeed;
    const auto rangeLifetime = CombatTime{static_cast<int64_t>(std::ceil(static_cast<double>(trap.range) * 1000.0 / speed))};
    const auto lifetime = std::min(rangeLifetime, CombatTime{context.config.flameLifetimeMs});
    const auto id = context.entityIds.generate();
    projectiles.emplace(id,
        Projectile{ProjectileData{id, origin, FlameType::Common}, static_cast<double>(origin.getX()), static_cast<double>(origin.getY()), speed * (step.getX() - origin.getX()),
            speed * (step.getY() - origin.getY()), context.config.trapDamage, context.time + lifetime, context.time});
}

void projectile_combat::update(CombatContext& context, DungeonRoom& room, const std::vector<Combatant>& players, CombatTime elapsed) {
    auto& projectiles = room.getCombat().projectiles;
    for (auto it = projectiles.begin(); it != projectiles.end();) {
        if (advanceProjectile(context, it->second, room, players, elapsed)) {
            ++it;
        } else {
            it = projectiles.erase(it);
        }
    }
}
