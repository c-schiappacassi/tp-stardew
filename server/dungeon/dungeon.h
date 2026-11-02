#pragma once

#include <cstdint>
#include <map>
#include <random>
#include <optional>
#include "dungeon_room.h"
#include "dungeon_config.h"
#include <chrono>
#include <vector>
#include "world/zone_map.h"

class Player;
class EntityIdGenerator;

struct DungeonRoomLayout {
    ZoneMap terrain;
    std::vector<Position> mineralSpawns;
    DungeonRoomPassages passages;
};

class Dungeon {
private:
    std::map<uint8_t, DungeonRoom> rooms;
    DungeonConfig config;
    std::mt19937 generator;
    const MineralDefinition* chooseMineral(uint8_t roomNumber);
public:
    explicit Dungeon(const DungeonConfig& config = {},uint32_t seed = std::random_device{}());
    bool spawnMineral(EntityId id,uint8_t roomNumber,const Position& pos);
    std::optional<Stack> rollMiningReward(ResourceType type);
    DungeonRoom* findRoom(uint8_t number);
    const DungeonRoom* findRoom(uint8_t number) const;
    const std::map<uint8_t, DungeonRoom>& getRooms() const;
    const DungeonConfig& getConfig() const;
    bool mineMineral(Player& player,EntityId mineralId,std::chrono::steady_clock::time_point now =std::chrono::steady_clock::now());
    std::map<uint8_t, std::vector<MineralData>> getMineralData(bool includeBroken = false) const;
    void removeBrokenMinerals();
    bool initialize(const std::map<uint8_t, DungeonRoomLayout>& layout,EntityIdGenerator& entityIds);
};