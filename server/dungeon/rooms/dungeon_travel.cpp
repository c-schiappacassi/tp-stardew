#include "dungeon_travel.h"

#include "player/player_manager.h"

std::optional<WorldLocation> DungeonTravel::spawnFor(const PlayerSnapshot& player, const DungeonRooms& rooms) const {
    const auto* first = rooms.find(1);
    if (!first || !first->getPassages() || !first->canWalk(player.getCollisionBounds(first->getPassages()->entry)))
        return std::nullopt;
    return WorldLocation{Zone::Dungeon, 1, std::nullopt, first->getPassages()->entry};
}

bool DungeonTravel::enter(PlayerManager& players, uint16_t playerId, const DungeonRooms& rooms) const {
    const auto player = players.snapshot(playerId);
    if (!player.canAct() || player.getZone() == Zone::Dungeon)
        return false;
    const auto spawn = spawnFor(player, rooms);
    return spawn && players.apply(playerId, player_action::RestoreLocation{spawn->zone, spawn->level, spawn->position, spawn->interior}).applied;
}

bool DungeonTravel::advanceRoom(PlayerManager& players, uint16_t playerId, const DungeonRooms& rooms) const {
    const auto player = players.snapshot(playerId);
    if (player.getZone() != Zone::Dungeon || !player.canAct()) {
        return false;
    }

    const auto* room = rooms.find(player.getDungeonLevel());
    if (!room || !room->getPassages() || !room->getPassages()->nextRoom) {
        return false;
    }

    const auto& passages = *room->getPassages();
    if (!room->getMap().sameBaseCell(player.getPosition(), *passages.nextRoom)) {
        return false;
    }

    const auto* next = rooms.find(player.getDungeonLevel() + 1);
    if (!next || !next->getPassages() || !next->canWalk(player.getCollisionBounds(next->getPassages()->entry))) {
        return false;
    }

    return players.apply(playerId, player_action::ChangeLocation{Zone::Dungeon, next->getNumber(), next->getPassages()->entry}).applied;
}

bool DungeonTravel::canExit(const PlayerSnapshot& player, const DungeonRooms& rooms) const {
    if (player.getZone() != Zone::Dungeon || !player.canAct()) {
        return false;
    }

    const auto* room = rooms.find(player.getDungeonLevel());
    return room && room->getPassages() && room->getMap().sameBaseCell(player.getPosition(), room->getPassages()->exit);
}

bool DungeonTravel::teleportToRoom(PlayerManager& players, uint16_t playerId, uint8_t number, const DungeonRooms& rooms) const {
    const auto player = players.snapshot(playerId);
    const auto* room = rooms.find(number);
    if (!room || !room->getPassages() || !room->canWalk(player.getCollisionBounds(room->getPassages()->entry)))
        return false;
    return players.apply(playerId, player_action::ChangeLocation{Zone::Dungeon, number, room->getPassages()->entry}).applied;
}
