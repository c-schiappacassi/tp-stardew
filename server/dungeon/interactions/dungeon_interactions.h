#pragma once

#include <chrono>
#include <cstdint>

#include "entities/entity_id.h"

#include "protocol_types.h"

class DungeonRooms;
class DungeonRoom;
class DungeonMining;
class MineralGenerator;
class DungeonCombat;
class PlayerManager;
class EntityIdGenerator;
struct MiningResult;
struct ItemTransferDTO;
struct SelectItemDTO;

class DungeonInteractions {
private:
    DungeonRooms& rooms;
    DungeonMining& mining;
    MineralGenerator& generator;
    DungeonCombat& combat;

    bool completeMining(DungeonRoom& room, const MiningResult& result) const;

public:
    DungeonInteractions(DungeonRooms& rooms, DungeonMining& mining, MineralGenerator& generator, DungeonCombat& combat);
    bool mineMineral(PlayerManager& players, uint16_t playerId, EntityId id, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) const;
    bool useSelectedItem(PlayerManager& players, uint16_t playerId, Direction direction, std::chrono::steady_clock::time_point now, EntityIdGenerator& ids) const;
    bool collectNearbyItems(PlayerManager& players, uint16_t playerId) const;
    bool depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command) const;
    bool withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command) const;
};
