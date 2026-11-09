#include "dungeon_mining.h"
#include "dungeon/rooms/dungeon_room.h"
#include "mineral_generator.h"
#include "player/player.h"
#include <cmath>
#include <limits>

DungeonMining::DungeonMining(uint16_t range) : range(range) {}

bool DungeonMining::mine(Player& player, DungeonRoom& room, EntityId id,MineralGenerator& generator, std::chrono::steady_clock::time_point now) const {
    if (player.getZone() != Zone::Dungeon || !player.canUsePickaxe(now)) {
        return false;
    }
    auto* mineral = room.findMineral(id);
    if (!mineral || mineral->isDepleted() ||!player.canReach(mineral->getPosition(), range) ||!room.hasClearLine(player.getCollisionCenter(), mineral->getCollisionCenter())) {
        return false;
    }
    player.faceTarget(mineral->getPosition());
    if (!player.useTool(now)) {
        return false;
    }
    if (mineral->hit()) {
        const auto reward = generator.rollReward(mineral->getType());
        if (reward) {
            player.addItem(*reward);
        }
    }
    return true;
}

EntityId DungeonMining::chooseMineral(const Player& player,const DungeonRoom& room,Direction direction) const {
    EntityId chosen = 0;
    double nearest = std::numeric_limits<double>::max();
    for (const auto& [id, mineral] : room.getMinerals()) {
        if (mineral.isDepleted() ||!player.canReach(mineral.getPosition(), range) ||!room.hasClearLine(player.getCollisionCenter(), mineral.getCollisionCenter())) {
            continue;
        }
        const double dx = static_cast<double>(mineral.getPosition().getX()) - player.getPosition().getX();
        const double dy = static_cast<double>(mineral.getPosition().getY()) - player.getPosition().getY();
        const bool ahead =
            (direction == Direction::Up && dy < 0 && std::abs(dx) <= -dy) ||
            (direction == Direction::Down && dy > 0 && std::abs(dx) <= dy) ||
            (direction == Direction::Left && dx < 0 && std::abs(dy) <= -dx) ||
            (direction == Direction::Right && dx > 0 && std::abs(dy) <= dx);
        const double distance = dx * dx + dy * dy;
        if (ahead && distance < nearest) {
            nearest = distance;
            chosen = id;
        }
    }
    return chosen;
}

bool DungeonMining::useSelectedItem(Player& player,DungeonRoom& room,MineralGenerator& generator,Direction direction,std::chrono::steady_clock::time_point now) const {
    if (player.getZone() != Zone::Dungeon || !player.canUsePickaxe(now)) {
        return false;
    }
    const EntityId chosen = chooseMineral(player, room, direction);
    return chosen != 0 && mine(player, room, chosen, generator, now);
}
