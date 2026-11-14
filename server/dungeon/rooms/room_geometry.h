#pragma once

#include <cstdint>
#include "dungeon_room_passages.h"
#include "room_minerals.h"

class RoomGeometry {
private:
    bool isPointClear(const ZoneMap& map, const RoomMinerals& minerals,const Position& point) const;

public:
    bool canWalk(const ZoneMap& map, const RoomMinerals& minerals,const CollisionBounds& bounds) const;
    bool isPassageCell(const ZoneMap& map, const DungeonRoomPassages& data,const Position& pos) const;
    bool arePassagesValid(uint8_t number, const ZoneMap& map,const DungeonRoomPassages& data) const;
    bool hasClearLine(const ZoneMap& map, const RoomMinerals& minerals,const Position& from, const Position& target) const;
};
