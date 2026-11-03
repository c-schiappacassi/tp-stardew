#pragma once

#include <cstdint>
#include "dungeon_room_passages.h"
#include "room_minerals.h"

class RoomGeometry {
public:
    bool canWalk(const ZoneMap& map, const RoomMinerals& minerals,const Position& pos) const;
    bool canWalk(const ZoneMap& map, const RoomMinerals& minerals,const CollisionBounds& bounds) const;
    bool isPassageCell(const ZoneMap& map, const DungeonRoomPassages& data,const Position& pos) const;
    bool arePassagesValid(uint8_t number, const ZoneMap& map,const DungeonRoomPassages& data) const;
    bool hasClearLine(const ZoneMap& map, const RoomMinerals& minerals,const Position& from, const Position& target) const;
};
