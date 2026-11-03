#pragma once

#include <chrono>
#include <cstdint>
#include "entities/entity_id.h"

class Player;
class DungeonRoom;
class MineralGenerator;

class DungeonMining {
private:
    uint16_t range;
    EntityId chooseMineral(const Player& player, const DungeonRoom& room) const;

public:
    explicit DungeonMining(uint16_t range);
    bool mine(Player& player, DungeonRoom& room, EntityId id,MineralGenerator& generator,std::chrono::steady_clock::time_point now) const;
    bool useSelectedItem(Player& player, DungeonRoom& room,MineralGenerator& generator,std::chrono::steady_clock::time_point now) const;
};