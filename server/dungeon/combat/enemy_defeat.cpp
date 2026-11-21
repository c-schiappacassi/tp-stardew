#include "enemy_defeat.h"

#include "combat_context.h"
#include "dungeon/rooms/dungeon_room.h"

void enemy_defeat::reward(CombatContext& context, uint16_t playerId, const Enemy& enemy,DungeonRoom& room) {
    context.progress.defeat(playerId, room.getNumber(), enemy);
    if (context.generator.roll(context.config.goldChance)) {
        room.getGroundItems().dropGold(enemy.data.pos, context.generator.gold(), context.entityIds);
    }
    if (enemy.data.type == EnemyType::Orc &&
        context.generator.roll(context.config.orcSilverChance)) {
        room.getGroundItems().drop(enemy.data.pos, ItemId::Silver, 1, Quality::None,context.entityIds);
    }
}
