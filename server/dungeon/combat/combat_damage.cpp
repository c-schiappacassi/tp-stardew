#include "combat_damage.h"

#include "dungeon/enemies/enemy_navigation.h"
#include "dungeon/rooms/dungeon_room.h"
#include "player/player.h"

#include "combat_context.h"

namespace {
void scheduleVampireRevival(const CombatContext& context, const Player& player, DungeonRoom& room) {
    const auto radius = context.config.bloodRadius;
    const double squaredRadius = static_cast<double>(radius) * radius;
    for (auto& [id, enemy]: room.getCombat().enemies) {
        if (enemy.alive() || enemy.pairId == 0 || enemy.reviveAt) {
            continue;
        }
        const auto distance =
                enemy_navigation::squaredDistance(enemy.data.pos, player.getPosition());
        if (distance <= squaredRadius) {
            enemy.reviveAt = context.time + CombatTime{context.config.vampireReviveMs};
        }
    }
}
}

void combat_damage::apply(CombatContext& context, Player& player, uint16_t playerId,
                          uint16_t amount, DungeonRoom& room) {
    const auto before = player.getLocalData();
    if (player.receiveDamage(amount, context.actionTime) == 0) {
        return;
    }
    context.progress.damage(playerId);

    const auto after = player.getLocalData();
    const bool crossedBloodThreshold =
            static_cast<uint32_t>(before.health) * 5 > before.maxHealth &&
            static_cast<uint32_t>(after.health) * 5 <= after.maxHealth;
    if (after.health == 0 || crossedBloodThreshold) {
        scheduleVampireRevival(context, player, room);
    }
}
