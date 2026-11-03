#include "mineral_placement.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <set>
#include "dungeon/rooms/dungeon_room.h"
#include "dungeon/rooms/room_geometry.h"

std::optional<std::vector<Position>> MineralPlacement::choose(const DungeonRoom& room,uint8_t densityPercent,const std::vector<Position>& reserved,std::mt19937& random) const {const auto& map = room.getMap();const auto& passages = room.getPassages();
    if (densityPercent > 100 || !passages || map.isEmpty()) {
        return std::nullopt;
    }

    std::set<ZoneMap::Cell> forbidden;
    for (const auto& position : reserved) {
        if (!map.isWalkable(position)) {
            return std::nullopt;
        }
        forbidden.insert(*map.cellAt(position));
    }

    const RoomGeometry geometry;
    std::vector<Position> candidates;

    for (const auto& position : map.walkablePositions()) {
        if (room.canWalk(position) &&
            !geometry.isPassageCell(map, *passages, position) &&
            !forbidden.contains(*map.cellAt(position))) {
            candidates.push_back(position);
        }
    }

    const std::size_t count =
        (candidates.size() / 100) * densityPercent +
        ((candidates.size() % 100) * densityPercent) / 100;

    if (count > std::numeric_limits<uint16_t>::max()) {
        return std::nullopt;
    }
    if (count == 0) {
        return std::vector<Position>{};
    }

    std::shuffle(candidates.begin(), candidates.end(), random);
    candidates.erase(
        candidates.begin() + static_cast<std::ptrdiff_t>(count),
        candidates.end());

    for (const auto& position : candidates) {
        if (position.getX() > std::numeric_limits<uint16_t>::max() ||position.getY() > std::numeric_limits<uint16_t>::max()) {
            return std::nullopt;
        }
    }
    return candidates;
}