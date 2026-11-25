#include "town_map.h"

#include "server_constants.h"

#include <algorithm>
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

    std::vector<std::vector<bool>> tiles;
    if (!config.walkableTiles.empty()) {
        if (config.walkableTiles.size() != map.height) return false;
        for (const auto& row : config.walkableTiles) {
            if (row.size() != map.width) return false;
        }
        tiles = config.walkableTiles;
    } else {
        tiles.assign(map.height, std::vector<bool>(map.width, true));
    }
    std::vector<std::vector<bool>> buildingFootprints(
        map.height, std::vector<bool>(map.width, false));
    for (const auto& building: config.buildings) {
        const auto right = static_cast<uint32_t>(building.position.x) + building.width;
        const auto bottom = static_cast<uint32_t>(building.position.y) + building.height;
        if (building.width == 0 || building.height == 0 || right > map.width || bottom > map.height) {
            return false;
        }
        if (building.door.size() > 1) return false;
        if (!building.door.empty()) {
            const TownPosition& door = building.door.front();
            if (door.x < building.position.x || door.x >= right ||
                door.y < building.position.y || door.y >= bottom) {
                return false;
            }
        }
        for (uint32_t row = building.position.y; row < bottom; ++row) {
            for (uint32_t col = building.position.x; col < right; ++col) {
                if (buildingFootprints[row][col]) return false;
                buildingFootprints[row][col] = true;
                if (!building.door.empty() && building.door.front().x == col &&
                    building.door.front().y == row) {
                    if (!tiles[row][col]) return false;
                    continue;
                }
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
        if (connection.destination == server_constants::FARM && !toFarm) {
            toFarm = base;
        } else if (connection.destination == server_constants::DUNGEON && !toMine) {
            toMine = base;
        } else {
            return false;
        }
    }
    std::vector<Position> extraFarm;
    for (const auto& extra: config.extraFarmDoors) {
        const ZoneMap::Cell cell{extra.y, extra.x};
        const auto base = prepared.baseAt(cell);
        if (!base || !prepared.isWalkable(playerBox.at(*base)) || !cells.insert(cell).second) {
            return false;
        }
        extraFarm.push_back(*base);
    }
    if (!toFarm || (toMine && !prepared.hasBasePath(*toFarm, *toMine))) return false;

    terrain = std::move(prepared);
    farmDoor = toFarm;
    farmDoorExtras = std::move(extraFarm);
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

bool TownMap::isFarmDoor(const Position& position) const {
    if (!farmDoor) return false;
    return terrain.sameBaseCell(position, *farmDoor) ||
           std::any_of(farmDoorExtras.begin(), farmDoorExtras.end(),
                       [&](const Position& door) { return terrain.sameBaseCell(position, door); });
}

std::optional<Position> TownMap::connectionTo(Zone destination) const {
    if (destination == Zone::Farm) return farmDoor;
    if (destination == Zone::Dungeon) return mineDoor;
    return std::nullopt;
}
