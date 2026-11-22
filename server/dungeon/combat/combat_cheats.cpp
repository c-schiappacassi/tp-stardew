#include "dungeon_combat.h"

#include <limits>

#include "combat_context.h"
#include "dungeon/enemies/enemy_navigation.h"
#include "enemy_behavior.h"
#include "enemy_defeat.h"
#include "player/player.h"

bool DungeonCombat::prepareVisit(Player& player, uint16_t playerId,std::map<uint8_t, DungeonRoom>& rooms, EntityIdGenerator& ids) {
    if (!visitEnded) return true;
    if (!regenerate(rooms, ids)) return false;
    onTravel(playerId, player.getDungeonLevel(), player.canBeAttacked(), rooms);
    return true;
}

bool DungeonCombat::spawnEnemies(DungeonRoom& room, EnemyType type, uint16_t quantity,bool pairs, EntityIdGenerator& ids,const std::vector<Position>& occupied) {
    const auto layout = layouts.find(room.getNumber());
    if (layout == layouts.end()) return false;
    auto reserved = layout->second.reservedPositions;
    reserved.insert(reserved.end(), occupied.begin(), occupied.end());
    if (!generator.spawnEnemies(room, type, quantity, pairs, ids, reserved)) return false;
    if (type == EnemyType::Skull) {
        for (const auto& [id, number] : presence) {
            if (number == room.getNumber()) progress.observeSkull(id, number);
        }
    }
    return true;
}

uint16_t DungeonCombat::killEnemies(Player& player, uint16_t playerId, DungeonRoom& room,bool nearestOnly, EntityIdGenerator& ids) {
    Enemy* nearest = nullptr;
    if (nearestOnly) {
        double distance = std::numeric_limits<double>::max();
        for (auto& [id, enemy] : room.getCombat().enemies) {
            const auto candidate = enemy_navigation::squaredDistance(player.getPosition(), enemy.data.pos);
            if (enemy.alive() && candidate < distance) {
                nearest = &enemy;
                distance = candidate;
            }
        }
    }
    CombatContext context{config, generator, progress, ids, clock, std::chrono::steady_clock::now()};
    uint16_t killed = 0;
    for (auto& [id, enemy] : room.getCombat().enemies) {
        if ((!nearestOnly || &enemy == nearest) && enemy.kill(clock, config)) {
            enemy_defeat::reward(context, playerId, enemy, room);
            ++killed;
        }
    }
    enemy_behavior::refreshVampirePairs(room);
    return killed;
}
