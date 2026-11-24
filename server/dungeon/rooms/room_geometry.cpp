#include "room_geometry.h"

#include <algorithm>
#include <cmath>

#include "dungeon/dungeon_config.h"

bool RoomGeometry::isPointClear(const ZoneMap& map, const RoomMinerals& minerals, const Position& pos) const {
    return canWalk(map, minerals, CollisionBounds{pos.getX(), pos.getY(), pos.getX(), pos.getY()});
}

bool RoomGeometry::canWalk(const ZoneMap& map, const RoomMinerals& minerals, const CollisionBounds& bounds) const { return map.isWalkable(bounds) && !minerals.blocks(bounds); }

bool RoomGeometry::isPassageCell(const ZoneMap& map,const DungeonRoomPassages& data, const Position& pos) const {
    return map.sameBaseCell(pos, data.entry) || map.sameBaseCell(pos, data.exit) ||(data.nextRoom && map.sameBaseCell(pos, *data.nextRoom)) ||
           std::any_of(data.extraExits.begin(), data.extraExits.end(),
                       [&](const Position& door) { return map.sameBaseCell(pos, door); });
}

bool RoomGeometry::arePassagesValid(uint8_t number, const ZoneMap& map,const DungeonRoomPassages& data) const {
    if (!map.isWalkableBase(data.entry) || !map.isWalkableBase(data.exit)) {
        return false;
    }
    if ((number < DUNGEON_ROOM_COUNT && !data.nextRoom) || (number == DUNGEON_ROOM_COUNT && data.nextRoom)) {
        return false;
    }
    if (data.nextRoom && (!map.isWalkableBase(*data.nextRoom) || map.sameBaseCell(data.entry, *data.nextRoom) || map.sameBaseCell(data.exit, *data.nextRoom))) {
        return false;
    }
    return map.hasBasePath(data.entry, data.exit) && (!data.nextRoom || map.hasBasePath(data.entry, *data.nextRoom));
}

bool RoomGeometry::hasClearLine(const ZoneMap& map, const RoomMinerals& minerals, const Position& from, const Position& target) const {
    if (!isPointClear(map, minerals, from) || !map.isWalkable(target)) {
        return false;
    }
    const double dx = static_cast<double>(target.getX()) - from.getX();
    const double dy = static_cast<double>(target.getY()) - from.getY();
    const int steps = static_cast<int>(std::max(std::abs(dx), std::abs(dy)));
    if (steps == 0) {
        return true;
    }

    Position previous = from;
    for (int step = 1; step <= steps; ++step) {
        const Position sample(static_cast<int>(std::round(from.getX() + dx * step / steps)), static_cast<int>(std::round(from.getY() + dy * step / steps)));
        const auto previousCell = map.cellAt(previous);
        const auto sampleCell = map.cellAt(sample);
        if (!previousCell || !sampleCell) {
            return false;
        }

        const bool crossesCorner = previousCell->first != sampleCell->first && previousCell->second != sampleCell->second;
        if (crossesCorner) {
            const Position horizontal(sample.getX(), previous.getY());
            const Position vertical(previous.getX(), sample.getY());
            if (!isPointClear(map, minerals, horizontal) || !isPointClear(map, minerals, vertical)) {
                return false;
            }
        }
        if (map.sameCell(sample, target)) {
            return true;
        }
        if (!isPointClear(map, minerals, sample)) {
            return false;
        }
        previous = sample;
    }
    return true;
}
