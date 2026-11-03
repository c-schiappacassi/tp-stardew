#pragma once

#include <cstdint>
#include <map>
#include "room_minerals.h"
#include "world/zone_map.h"
#include "dungeon_room_passages.h"
#include "room_geometry.h"
#include <optional>



class DungeonRoom {
private:
    uint8_t number;
    ZoneMap terrain;
    RoomMinerals minerals;
    RoomGeometry geometry;
    std::optional<DungeonRoomPassages> passages;

public:
    explicit DungeonRoom(uint8_t number);

    uint8_t getNumber() const;
    bool addMineral(Mineral mineral);
    bool removeMineral(EntityId id);
    Mineral* findMineral(EntityId id);
    const Mineral* findMineral(EntityId id) const;
    const std::map<EntityId, Mineral>& getMinerals() const;
    bool setMap(const ZoneMap& map);
    const ZoneMap& getMap() const;
    bool canWalk(const Position& pos) const;
    bool canWalk(const CollisionBounds& bounds) const;
    void removeBrokenMinerals();
    bool setPassages(const DungeonRoomPassages& data);
    const std::optional<DungeonRoomPassages>& getPassages() const;
    bool hasClearLine(const Position& from,const Position& target) const;
    std::optional<MineData> getMineData(bool includeBroken = false) const;
};
