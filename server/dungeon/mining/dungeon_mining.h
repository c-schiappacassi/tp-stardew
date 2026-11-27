#pragma once

#include <chrono>
#include <cstdint>

#include "entities/entity_id.h"

#include "protocol_types.h"

#include "dungeon/dungeon_types.h"

class PlayerManager;
class PlayerSnapshot;
class DungeonRoom;
class MineralGenerator;
class EntityIdGenerator;

class DungeonMining {
private:
    uint16_t range;
    EntityId chooseMineral(const PlayerSnapshot& player, const DungeonRoom& room, Direction direction) const;

public:
    explicit DungeonMining(uint16_t range);

    MiningResult mine(PlayerManager& players, uint16_t playerId, DungeonRoom& room, EntityId id, MineralGenerator& generator, EntityIdGenerator& ids,
        std::chrono::steady_clock::time_point now) const;
    MiningResult useSelectedItem(PlayerManager& players, uint16_t playerId, DungeonRoom& room, MineralGenerator& generator, EntityIdGenerator& ids, Direction direction,
        std::chrono::steady_clock::time_point now) const;
};
