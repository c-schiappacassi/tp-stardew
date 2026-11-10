#pragma once

#include <chrono>
#include <cstdint>
#include "entities/entity_id.h"
#include "protocol_types.h"

class Player;
class DungeonRoom;
class MineralGenerator;

struct MiningResult {
    bool acted = false;
    bool obtainedSilver = false;
};

class DungeonMining {
private:
    uint16_t range;
    EntityId chooseMineral(const Player& player, const DungeonRoom& room,Direction direction) const;

public:
    explicit DungeonMining(uint16_t range);

    MiningResult mine(Player& player,DungeonRoom& room,EntityId id,MineralGenerator& generator,std::chrono::steady_clock::time_point now) const;
    MiningResult useSelectedItem(Player& player, DungeonRoom& room, MineralGenerator& generator,Direction direction,std::chrono::steady_clock::time_point now) const;
};