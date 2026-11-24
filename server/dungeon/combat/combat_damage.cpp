#include "combat_damage.h"

#include "dungeon/dungeon_room.h"
#include "dungeon/enemies/enemy_navigation.h"
#include "player/player_manager.h"

#include "combat_context.h"

namespace {
void scheduleVampireRevival(const CombatContext& context, const PlayerSnapshot& player, DungeonRoom& room) {
    const auto radius = context.config.bloodRadius;
    const double squaredRadius = static_cast<double>(radius) * radius;
    for (auto& [id, enemy]: room.getCombat().enemies) {
        if (!enemy.needsBloodRevival()) {
            continue;
        }
        const auto distance = enemy_navigation::squaredDistance(enemy.getPosition(), player.getPosition());
        if (distance <= squaredRadius) {
            enemy.receiveBlood(context.time, context.config);
        }
    }
}
}  // namespace

void combat_damage::apply(CombatContext& context, uint16_t playerId, uint16_t amount, DungeonRoom& room) {
    auto& players = context.players;
    const auto player = players.snapshot(playerId);
    const auto before = player.getVitals();
    if (players.apply(playerId, player_action::ReceiveDamage{amount, context.actionTime}).amount == 0) {
        return;
    }
    context.progress.damage(playerId);

    const auto after = players.snapshot(playerId).getVitals();
    const bool crossedBloodThreshold = static_cast<uint32_t>(before.health) * 5 > before.maxHealth && static_cast<uint32_t>(after.health) * 5 <= after.maxHealth;
    if (after.health == 0 || crossedBloodThreshold) {
        scheduleVampireRevival(context, player, room);
    }
}
