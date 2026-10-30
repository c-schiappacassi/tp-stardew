#pragma once

#include <cstdint>
#include <map>
#include <random>
#include "dungeon_room.h"
#include "dungeon_config.h"

class Dungeon {
private:
    std::map<uint8_t, DungeonRoom> rooms;
    DungeonConfig config;
    std::mt19937 generator;
    const MineralDefinition* chooseMineral(uint8_t roomNumber);
public:
    explicit Dungeon(const DungeonConfig& config = {},uint32_t seed = std::random_device{}());
    bool spawnMineral(EntityId id,uint8_t roomNumber,const Position& pos);
    DungeonRoom* findRoom(uint8_t number);
    const DungeonRoom* findRoom(uint8_t number) const;
    const std::map<uint8_t, DungeonRoom>& getRooms() const;
};