#include "town_map.h"

#include <cstdint>
#include <limits>
#include <set>
#include <utility>
#include <vector>

#include "entities/collision_box.h"
#include "town_config.h"

bool TownMap::initialize(const TownConfig& config) {
    const auto& map = config.map;
    const auto limit = std::numeric_limits<uint16_t>::max();
    if (map.width == 0 || map.height == 0 || map.tileSize < entity_dimensions::PLAYER.width ||
        static_cast<uint32_t>(map.width) * map.tileSize > limit ||
        static_cast<uint32_t>(map.height) * map.tileSize > limit ||
        static_cast<uint32_t>(map.width) * map.height > 1024 * 1024) {
        return false;
    }

    std::vector<std::vector<bool>> tiles(map.height, std::vector<bool>(map.width, true));
    for (const auto& building: config.buildings) {
        const auto right = static_cast<uint32_t>(building.position.x) + building.width;
        const auto bottom = static_cast<uint32_t>(building.position.y) + building.height;
        if (building.width == 0 || building.height == 0 || right > map.width || bottom > map.height) {
            return false;
        }
        for (uint32_t row = building.position.y; row < bottom; ++row) {
            for (uint32_t col = building.position.x; col < right; ++col) {
                if (!tiles[row][col]) return false;
                tiles[row][col] = false;
            }
        }
    }
    for (const auto& npc: config.npcs) {
        if (npc.position.x >= map.width || npc.position.y >= map.height ||
            !tiles[npc.position.y][npc.position.x]) return false;
        tiles[npc.position.y][npc.position.x] = false;
    }

    ZoneMap prepared{tiles, map.tileSize};
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    std::optional<Position> toFarm;
    std::optional<Position> toMine;
    std::set<ZoneMap::Cell> cells;
    for (const auto& connection: config.connections) {
        const ZoneMap::Cell cell{connection.position.y, connection.position.x};
        const auto base = prepared.baseAt(cell);
        if (!base || !prepared.isWalkable(playerBox.at(*base)) || !cells.insert(cell).second) {
            return false;
        }
        if (connection.destination == "farm" && !toFarm) {
            toFarm = base;
        } else if (connection.destination == "dungeon" && !toMine) {
            toMine = base;
        } else {
            return false;
        }
    }
    if (!toFarm || (toMine && !prepared.hasBasePath(*toFarm, *toMine))) return false;

    terrain = std::move(prepared);
    farmDoor = toFarm;
    mineDoor = toMine;
    widthTiles = map.width;
    heightTiles = map.height;
    tileSizePx = map.tileSize;
    return true;
}

const ZoneMap& TownMap::getMap() const { return terrain; }

bool TownMap::canWalk(const CollisionBounds& bounds) const {
    return !terrain.isEmpty() && terrain.isWalkable(bounds);
}

std::optional<Position> TownMap::connectionTo(Zone destination) const {
    if (destination == Zone::Farm) return farmDoor;
    if (destination == Zone::Dungeon) return mineDoor;
    return std::nullopt;
}
