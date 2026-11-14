#pragma once

#include <map>
#include "dungeon_room.h"

class Player;

class DungeonTravel {
private:
    static const DungeonRoom* findRoom(const std::map<uint8_t, DungeonRoom>& rooms, uint8_t number);

public:
    bool spawnPlayer(Player& player, const std::map<uint8_t, DungeonRoom>& rooms) const;
    bool enter(Player& player,const std::map<uint8_t, DungeonRoom>& rooms) const;
    bool advanceRoom(Player& player,const std::map<uint8_t, DungeonRoom>& rooms) const;
    bool canExit(const Player& player,const std::map<uint8_t, DungeonRoom>& rooms) const;
};
