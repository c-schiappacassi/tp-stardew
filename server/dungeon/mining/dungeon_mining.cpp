#include "dungeon_mining.h"

#include <cmath>
#include <limits>

#include "dungeon/rooms/dungeon_room.h"
#include "player/player.h"

#include "mineral_generator.h"

DungeonMining::DungeonMining(uint16_t range): range(range) {}

MiningResult DungeonMining::mine(Player& player, DungeonRoom& room, EntityId id,MineralGenerator& generator, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) const {
    if (player.getZone() != Zone::Dungeon || player.getDungeonLevel() != room.getNumber() ||
        !player.canUsePickaxe(now)) {
        return MiningResult();
    }
    auto* mineral = room.findMineral(id);
    if (!mineral || mineral->isDepleted() || !player.canReach(mineral->getPosition(), range) ||
        !room.hasClearLine(player.getCollisionCenter(), mineral->getCollisionCenter())) {
        return MiningResult();
    }
    player.faceTarget(mineral->getPosition());
    if (!player.useTool(now)) {
        return MiningResult();
    }
    bool droppedSilver = false;
    auto preparedMineral = *mineral;
    if (preparedMineral.hit()) {
        const auto reward = generator.rollReward(mineral->getType());
        if (reward) {
            if (!room.getGroundItems().drop(mineral->getPosition(), reward->item_id,
                    reward->quantity, reward->qual, ids)) {
                return {true, false};
            }
            droppedSilver = reward->item_id == ItemId::Silver;
        }
    }
    *mineral = preparedMineral;
    return {true, droppedSilver};
}

EntityId DungeonMining::chooseMineral(const Player& player, const DungeonRoom& room,
                                      Direction direction) const {
    EntityId chosen = 0;
    double nearest = std::numeric_limits<double>::max();
    for (const auto& [id, mineral]: room.getMinerals()) {
        if (mineral.isDepleted() || !player.canReach(mineral.getPosition(), range) ||
            !room.hasClearLine(player.getCollisionCenter(), mineral.getCollisionCenter())) {
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

MiningResult DungeonMining::useSelectedItem(Player& player, DungeonRoom& room,MineralGenerator& generator, EntityIdGenerator& ids, Direction direction,std::chrono::steady_clock::time_point now) const {
    if (player.getZone() != Zone::Dungeon || !player.canUsePickaxe(now)) {
        return MiningResult();
    }
    const EntityId chosen = chooseMineral(player, room, direction);
    return chosen != 0 ? mine(player, room, chosen, generator, ids, now) : MiningResult{};
}
