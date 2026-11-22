#pragma once
#include <map>

#include "world/world_location.h"

#include "dungeon_room.h"

class PlayerSnapshot;
class PlayerManager;

class DungeonTravel {
private:
    static const DungeonRoom* findRoom(const std::map<uint8_t, DungeonRoom>& rooms, uint8_t number);

public:
    std::optional<WorldLocation> spawnFor(const PlayerSnapshot& player, const std::map<uint8_t, DungeonRoom>& rooms) const;
    bool enter(PlayerManager& players, uint16_t playerId, const std::map<uint8_t, DungeonRoom>& rooms) const;
    bool advanceRoom(PlayerManager& players, uint16_t playerId, const std::map<uint8_t, DungeonRoom>& rooms) const;
    bool canExit(const PlayerSnapshot& player, const std::map<uint8_t, DungeonRoom>& rooms) const;
};
