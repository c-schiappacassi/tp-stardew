#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

#include "protocol_types.h"

class DungeonManager;
class FarmManager;
class InteriorRooms;
class EntityIdGenerator;
class PlayerManager;
struct ItemTransferDTO;
struct SelectItemDTO;

class WorldInteractions {
private:
    DungeonManager& dungeon;
    FarmManager& farm;
    InteriorRooms& interiors;
    EntityIdGenerator& entityIds;

public:
    WorldInteractions(DungeonManager& dungeon, FarmManager& farm, InteriorRooms& interiors, EntityIdGenerator& ids);
    void useSelectedItem(PlayerManager& players, uint16_t playerId, Direction direction, std::chrono::steady_clock::time_point now);
    void depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command);
    void withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command);
    void collectNearbyItems(PlayerManager& players, const std::vector<uint16_t>& playerIds);
};
