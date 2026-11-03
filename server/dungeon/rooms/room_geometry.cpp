#include "room_geometry.h"
#include "dungeon/dungeon_config.h"
#include <algorithm>
#include <cmath>

bool RoomGeometry::canWalk(const ZoneMap& map, const RoomMinerals& minerals,const Position& pos) const {
    if (!map.isWalkable(pos)) {
        return false;
    }
    return !minerals.blocks(*map.cellAt(pos));
}

bool RoomGeometry::canWalk(const ZoneMap& map, const RoomMinerals& minerals,const CollisionBounds& bounds) const {
    if (!map.isWalkable(bounds)) return false;
    const auto cells = map.coveredCells(bounds);
    for (int row = cells->first.first; row <= cells->last.first; ++row) {
        for (int column = cells->first.second; column <= cells->last.second; ++column) {
            if (minerals.blocks({row, column})) return false;
        }
    }
    return true;
}

bool RoomGeometry::isPassageCell(const ZoneMap& map,const DungeonRoomPassages& data, const Position& pos) const {
    return map.sameCell(pos, data.entry) || map.sameCell(pos, data.exit) ||(data.nextRoom && map.sameCell(pos, *data.nextRoom));
}

bool RoomGeometry::arePassagesValid(uint8_t number, const ZoneMap& map,const DungeonRoomPassages& data) const {
    if (!map.isWalkable(data.entry) || !map.isWalkable(data.exit)) {
        return false;
    }
    if ((number < DUNGEON_ROOM_COUNT && !data.nextRoom)||(number == DUNGEON_ROOM_COUNT && data.nextRoom)) {
        return false;
    }
    if (data.nextRoom && (!map.isWalkable(*data.nextRoom) ||map.sameCell(data.entry, *data.nextRoom) ||map.sameCell(data.exit, *data.nextRoom))) {
        return false;
    }
    return map.hasPath(data.entry, data.exit) &&(!data.nextRoom || map.hasPath(data.entry, *data.nextRoom));
}

bool RoomGeometry::hasClearLine(const ZoneMap& map, const RoomMinerals& minerals,const Position& from, const Position& target) const {
    if (!canWalk(map, minerals, from) || !map.isWalkable(target)) {
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
        const Position sample(static_cast<int>(std::round(from.getX() + dx * step / steps)),static_cast<int>(std::round(from.getY() + dy * step / steps)));
        const auto previousCell = map.cellAt(previous);
        const auto sampleCell = map.cellAt(sample);
        if (!previousCell || !sampleCell) {
            return false;
        }

        const bool crossesCorner = previousCell->first != sampleCell->first &&previousCell->second != sampleCell->second;
        if (crossesCorner) {
            const Position horizontal(sample.getX(), previous.getY());
            const Position vertical(previous.getX(), sample.getY());
            if (!canWalk(map, minerals, horizontal) ||
                !canWalk(map, minerals, vertical)) {
                return false;
            }
        }
        if (map.sameCell(sample, target)) {
            return true;
        }
        if (!canWalk(map, minerals, sample)) {
            return false;
        }
        previous = sample;
    }
    return true;
}
