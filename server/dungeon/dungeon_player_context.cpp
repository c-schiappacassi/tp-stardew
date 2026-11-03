#include "dungeon_player_context.h"

#include "player/player.h"

DungeonPlayerContext::DungeonPlayerContext(Dungeon& dungeon,const std::optional<DungeonEntrance>& entrance,const std::optional<uint8_t>& loadedWorld):
    dungeon(dungeon), entrance(entrance), loadedWorld(loadedWorld) {}

bool DungeonPlayerContext::canMove(const Player& player, const Position& next) const {
    if (player.getZone() == Zone::Dungeon) {
        const DungeonRoom* room =dungeon.findRoom(player.getDungeonLevel());

        return room && room->canWalk(next);
    }

    return !entrance || player.getZone() != entrance->zone ||entrance->terrain.isWalkable(next);
}

void DungeonPlayerContext::completeZoneData(const Player& player,PlayerDataLocal& data,bool includeBroken) const {
    if (player.getZone() != Zone::Dungeon) {
        return;
    }

    if (const auto mine = dungeon.getMineData(player.getDungeonLevel(), includeBroken)) {
        data.zoneData = *mine;
    }
}

void DungeonPlayerContext::interact(Player& player, const InteractDTO& interaction) {
    if (interaction.targetType == TargetType::Ore) {
        dungeon.mineMineral(player, interaction.targetId);
    } else if (interaction.targetType == TargetType::DungeonPassage && interaction.targetId == 0) {
        usePassage(player);
    }
}

bool DungeonPlayerContext::usePassage(Player& player) {
    if (!loadedWorld || !entrance || !player.canAct()) {
        return false;
    }

    if (player.getZone() == Zone::Dungeon) {
        if (dungeon.canExit(player)) {
            return player.changeLocation(entrance->zone, 0, entrance->returnPosition);
        }
        return dungeon.advanceRoom(player);
    }

    if (player.getZone() != entrance->zone||!entrance->terrain.sameCell(player.getPosition(), entrance->position)) {
        return false;
    }

    return dungeon.enter(player);
}