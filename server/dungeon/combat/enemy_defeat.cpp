#include "enemy_defeat.h"

#include "dungeon/dungeon_room.h"

#include "combat_context.h"

void enemy_defeat::reward(CombatContext& context, uint16_t playerId, const Enemy& enemy, DungeonRoom& room) {
    context.progress.defeat(playerId, room.getVisitDepth(), enemy);
    if (context.generator.roll(context.config.goldChance)) {
        room.getGroundItems().dropGold(enemy.getPosition(), context.generator.gold(), context.entityIds);
    }
    if (enemy.getType() == EnemyType::Orc && context.generator.roll(context.config.orcSilverChance)) {
        room.getGroundItems().drop(enemy.getPosition(), ItemId::Silver, 1, Quality::None, context.entityIds);
    }
}
