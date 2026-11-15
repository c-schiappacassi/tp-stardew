#include "sword_combat.h"

#include <algorithm>
#include <limits>

#include "dungeon/enemies/enemy_navigation.h"
#include "dungeon/rooms/dungeon_room.h"
#include "player/player.h"

#include "combat_context.h"
#include "enemy_behavior.h"

namespace {
bool canAttack(const Player& player, const DungeonRoom& room, Direction direction,std::chrono::steady_clock::time_point now) {
    const auto* selected = player.getSelectedItem();
    return player.getZone() == Zone::Dungeon && player.getDungeonLevel() == room.getNumber() &&
           selected && selected->item_id == ItemId::Sword && player.canUseTool(now) &&
           direction != Direction::None &&
           static_cast<uint8_t>(direction) <= static_cast<uint8_t>(Direction::Right);
}

Enemy* findTarget(Player& player, DungeonRoom& room, Direction direction) {
    Enemy* nearestEnemy = nullptr;
    double nearestDistance = std::numeric_limits<double>::max();
    const auto origin = player.getCollisionCenter();
    const auto range = player.getSwordRange();
    const double squaredRange = static_cast<double>(range) * range;

    for (auto& [id, enemy]: room.getCombat().enemies) {
        if (!enemy.alive()) {
            continue;
        }
        const auto target = enemy.bounds(enemy.data.pos).center();
        const auto distance = enemy_navigation::squaredDistance(origin, target);
        if (distance <= squaredRange && distance < nearestDistance &&
            enemy_navigation::isInFront(direction, origin, target) &&
            room.hasClearLine(origin, target)) {
            nearestEnemy = &enemy;
            nearestDistance = distance;
        }
    }
    return nearestEnemy;
}

void dropSilver(CombatContext& context, const Enemy& enemy, DungeonRoom& room) {
    auto& drops = room.getCombat().loot;
    for (auto& [id, loot]: drops) {
        if (loot.item == ItemId::Silver && loot.pos.getX() == enemy.data.pos.getX() &&
            loot.pos.getY() == enemy.data.pos.getY() &&
            loot.quantity < std::numeric_limits<uint16_t>::max()) {
            ++loot.quantity;
            return;
        }
    }
    if (context.entityIds.getNextId() == 0) {
        return;
    }
    LootData loot{context.entityIds.generate(), enemy.data.pos, ItemId::Silver, 1};
    drops.emplace(loot.id, loot);
}

void rewardDefeat(CombatContext& context, Player& player, uint16_t playerId, const Enemy& enemy,DungeonRoom& room) {
    context.progress.defeat(playerId, room.getNumber(), enemy);
    if (context.generator.roll(context.config.goldChance)) {
        player.addGold(context.generator.gold());
    }
    if (enemy.data.type == EnemyType::Orc &&
        context.generator.roll(context.config.orcSilverChance)) {
        if (!player.addItem(Stack{ItemId::Silver, 1, Quality::None})) {
            dropSilver(context, enemy, room);
        }
    }
}
}  // namespace

bool sword_combat::attack(CombatContext& context, Player& player, uint16_t playerId,DungeonRoom& room, Direction direction) {
    if (!canAttack(player, room, direction, context.actionTime)) {
        return false;
    }
    Enemy* target = findTarget(player, room, direction);
    const auto damage = player.getSwordDamage();
    player.faceDirection(direction);
    if (!player.useTool(context.actionTime)) {
        return false;
    }
    if (!target || !target->hit(damage, context.time, context.config)) {
        return true;
    }
    rewardDefeat(context, player, playerId, *target, room);
    enemy_behavior::refreshVampirePairs(room);
    return true;
}

bool sword_combat::collectLoot(Player& player, DungeonRoom& room) {
    if (!player.canAct() || player.getZone() != Zone::Dungeon ||
        player.getDungeonLevel() != room.getNumber()) {
        return false;
    }
    auto& drops = room.getCombat().loot;
    for (auto it = drops.begin(); it != drops.end(); ++it) {
        auto& loot = it->second;
        if (!player.canReach(loot.pos, 24) ||
            !room.hasClearLine(player.getCollisionCenter(), loot.pos)) {
            continue;
        }
        for (uint16_t amount = std::min<uint16_t>(50, loot.quantity); amount > 0; --amount) {
            if (!player.addItem(Stack{loot.item, amount, Quality::None})) {
                continue;
            }
            loot.quantity -= amount;
            if (loot.quantity == 0) {
                drops.erase(it);
            }
            return true;
        }
        return false;
    }
    return false;
}
