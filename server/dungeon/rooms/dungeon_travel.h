#pragma once

#include "world/world_location.h"

#include "dungeon_rooms.h"

class PlayerSnapshot;
class PlayerManager;

class DungeonTravel {
public:
    std::optional<WorldLocation> spawnFor(const PlayerSnapshot& player, const DungeonRooms& rooms) const;
    bool enter(PlayerManager& players, uint16_t playerId, const DungeonRooms& rooms) const;
    bool advanceRoom(PlayerManager& players, uint16_t playerId, const DungeonRooms& rooms) const;
    bool teleportToRoom(PlayerManager& players, uint16_t playerId, uint8_t number, const DungeonRooms& rooms) const;
    bool canExit(const PlayerSnapshot& player, const DungeonRooms& rooms) const;
};
