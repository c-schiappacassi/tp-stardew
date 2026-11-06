#include "dungeon_travel.h"
#include "player/player.h"

const DungeonRoom* DungeonTravel::findRoom(const std::map<uint8_t, DungeonRoom>& rooms, uint8_t number) {
    const auto it = rooms.find(number);
    return it == rooms.end() ? nullptr : &it->second;
}

bool DungeonTravel::enter(Player& player, const std::map<uint8_t, DungeonRoom>& rooms) const {
    if (player.getZone() == Zone::Dungeon || !player.canAct()) {
        return false;
    }

    const auto* first = findRoom(rooms, 1);
    if (!first || !first->getPassages() ||
        !first->canWalk(player.getCollisionBounds(first->getPassages()->entry))) {
        return false;
    }

    return player.changeLocation(Zone::Dungeon, 1, first->getPassages()->entry);
}

bool DungeonTravel::advanceRoom(Player& player, const std::map<uint8_t, DungeonRoom>& rooms) const {
    if (player.getZone() != Zone::Dungeon || !player.canAct()) {
        return false;
    }

    const auto* room = findRoom(rooms, player.getDungeonLevel());
    if (!room || !room->getPassages() || !room->getPassages()->nextRoom) {
        return false;
    }

    const auto& passages = *room->getPassages();
    if (!room->getMap().sameBaseCell(player.getPosition(), *passages.nextRoom)) {
        return false;
    }

    const auto* next = findRoom(rooms, player.getDungeonLevel() + 1);
    if (!next || !next->getPassages() ||
        !next->canWalk(player.getCollisionBounds(next->getPassages()->entry))) {
        return false;
    }

    return player.changeLocation(Zone::Dungeon, next->getNumber(), next->getPassages()->entry);
}

bool DungeonTravel::canExit(const Player& player, const std::map<uint8_t, DungeonRoom>& rooms) const {
    if (player.getZone() != Zone::Dungeon || !player.canAct()) {
        return false;
    }

    const auto* room = findRoom(rooms, player.getDungeonLevel());
    return room && room->getPassages() && room->getMap().sameBaseCell(player.getPosition(), room->getPassages()->exit);
}
