#include "sword_combat.h"

#include <algorithm>
#include <limits>

#include "dungeon/enemies/enemy_navigation.h"
#include "dungeon/rooms/dungeon_room.h"
#include "player/player.h"

#include "combat_context.h"
#include "enemy_behavior.h"
#include "enemy_defeat.h"

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
    enemy_defeat::reward(context, playerId, *target, room);
    enemy_behavior::refreshVampirePairs(room);
    return true;
}
