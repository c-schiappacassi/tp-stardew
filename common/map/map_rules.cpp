#include "common/map/map_rules.h"

#include <algorithm>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <utility>

namespace map_rules {

namespace {

struct Grid {
    int width = 0;
    int height = 0;
    std::vector<bool> walkable;

    bool inside(int x, int y) const { return x >= 0 && y >= 0 && x < width && y < height; }
    bool isWalkable(int x, int y) const {
        return inside(x, y) && walkable[static_cast<std::size_t>(y * width + x)];
    }
};

struct Anchor {
    std::string label;
    TilePosition tile;
};

const char* shopName(BuildingId shop) {
    switch (shop) {
        case BuildingId::MarniesShop: return "Marnie's ranch";
        case BuildingId::PierresShop: return "Pierre's general store";
        case BuildingId::RobinsShop: return "Robin's carpenter shop";
        case BuildingId::ClintsShop: return "Clint's blacksmith";
        case BuildingId::ComunityCenter: return "Community center";
        default: return "building";
    }
}

std::string at(const TilePosition& tile) {
    return "(" + std::to_string(tile.x) + ", " + std::to_string(tile.y) + ")";
}

bool buildGrid(const EditorZoneData& zone, const std::string& name, Grid& grid, std::vector<std::string>& errors) {
    if (zone.width <= 0 || zone.height <= 0) {
        errors.push_back(name + ": invalid dimensions");
        return false;
    }
    grid.width = zone.width;
    grid.height = zone.height;
    grid.walkable.assign(static_cast<std::size_t>(zone.width) * static_cast<std::size_t>(zone.height), false);
    std::vector<bool> seen(grid.walkable.size(), false);

    int invalid = 0;
    for (const FloorTile& tile: zone.background) {
        if (!grid.inside(tile.x, tile.y) || seen[static_cast<std::size_t>(tile.y * grid.width + tile.x)]) {
            ++invalid;
            continue;
        }
        seen[static_cast<std::size_t>(tile.y * grid.width + tile.x)] = true;
        grid.walkable[static_cast<std::size_t>(tile.y * grid.width + tile.x)] = tile.walkable;
    }
    const auto missing = static_cast<int>(std::count(seen.begin(), seen.end(), false));
    if (invalid > 0) {
        errors.push_back(name + ": " + std::to_string(invalid) + " floor tiles are outside the map or duplicated");
    }
    if (missing > 0) {
        errors.push_back(name + ": " + std::to_string(missing) + " floor tiles are missing");
    }
    return invalid == 0 && missing == 0;
}

bool requireWalkable(const Grid& grid, const std::string& name, const std::string& label, const TilePosition& tile,
                     std::vector<std::string>& errors) {
    if (!grid.inside(tile.x, tile.y)) {
        errors.push_back(name + ": " + label + " " + at(tile) + " is outside the map");
        return false;
    }
    if (!grid.isWalkable(tile.x, tile.y)) {
        errors.push_back(name + ": " + label + " " + at(tile) + " is not walkable");
        return false;
    }
    return true;
}

std::optional<Anchor> checkPortal(const Grid& grid, const std::string& name, const char* key, PortalId portal,
                                  const std::optional<TilePosition>& door, std::vector<Anchor>& anchors,
                                  std::vector<std::string>& errors) {
    if (!door) {
        errors.push_back(name + ": missing " + key);
        return std::nullopt;
    }
    const Shape& shape = shapeOf(portal);
    const TilePosition origin = originOf(shape, *door);
    if (!grid.inside(origin.x, origin.y) || !grid.inside(origin.x + shape.width() - 1, origin.y + shape.height() - 1)) {
        errors.push_back(name + ": the portal of " + key + " " + at(*door) + " does not fit inside the map");
        return std::nullopt;
    }
    for (int dy = 0; dy < shape.height(); ++dy) {
        for (int dx = 0; dx < shape.width(); ++dx) {
            if (shape.blocks(dx, dy) && grid.isWalkable(origin.x + dx, origin.y + dy)) {
                errors.push_back(name + ": the portal of " + key + " " + at(*door) +
                                 " has walkable tiles where its shape is solid");
                dy = shape.height();
                break;
            }
        }
    }
    const std::vector<TilePosition> doors = doorsOf(shape, origin.x, origin.y);
    if (!requireWalkable(grid, name, key, doors.front(), errors)) {
        return std::nullopt;
    }
    const Anchor primary{key, doors.front()};
    anchors.push_back(primary);
    for (std::size_t extra = 1; extra < doors.size(); ++extra) {
        const std::string label = std::string(key) + " " + std::to_string(extra + 1);
        if (requireWalkable(grid, name, label, doors[extra], errors)) {
            anchors.push_back({label, doors[extra]});
        }
    }
    return primary;
}

std::optional<Anchor> checkFixedDoor(const Grid& grid, const std::string& name, const char* key,
                                     const std::optional<TilePosition>& door, bool required,
                                     std::vector<std::string>& errors) {
    if (!door) {
        if (required) {
            errors.push_back(name + ": missing " + key);
        }
        return std::nullopt;
    }
    if (!requireWalkable(grid, name, key, *door, errors)) {
        return std::nullopt;
    }
    return Anchor{key, *door};
}

template <typename IsAllowed>
void checkBuildings(const EditorZoneData& zone, const Grid& grid, const std::string& name, IsAllowed isAllowed,
                    std::vector<Anchor>& anchors, std::vector<std::string>& errors) {
    std::vector<int> owner(static_cast<std::size_t>(grid.width * grid.height), -1);

    for (std::size_t index = 0; index < zone.buildings.size(); ++index) {
        const EditorBuildingData& building = zone.buildings[index];
        const std::string label = "building " + std::to_string(index + 1) + " (id " + std::to_string(building.id) + ")";
        if (!isAllowed(building.id)) {
            errors.push_back(name + ": " + label + " is not allowed here");
            continue;
        }

        const Shape& shape = shapeOf(static_cast<BuildingId>(building.id));
        if (building.x < 0 || building.y < 0 || building.x + shape.width() > grid.width ||
            building.y + shape.height() > grid.height) {
            errors.push_back(name + ": " + label + " does not fit inside the map");
            continue;
        }

        const TilePosition expected = doorOf(shape, building.x, building.y);
        if (building.door.x != expected.x || building.door.y != expected.y) {
            errors.push_back(name + ": " + label + " has its door at " + at(building.door) +
                             " but its shape puts it at " + at(expected));
            continue;
        }

        bool overlaps = false;
        bool walkableBody = false;
        for (int dy = 0; dy < shape.height(); ++dy) {
            for (int dx = 0; dx < shape.width(); ++dx) {
                if (!shape.blocks(dx, dy)) {
                    continue;
                }
                const int x = building.x + dx;
                const int y = building.y + dy;
                int& tileOwner = owner[static_cast<std::size_t>(y * grid.width + x)];
                overlaps = overlaps || tileOwner != -1;
                tileOwner = static_cast<int>(index);
                walkableBody = walkableBody || grid.isWalkable(x, y);
            }
        }
        if (overlaps) {
            errors.push_back(name + ": " + label + " overlaps another building");
        }
        if (walkableBody) {
            errors.push_back(name + ": " + label + " has walkable tiles where its shape is solid");
        }

        const std::vector<TilePosition> doors = doorsOf(shape, building.x, building.y);
        for (std::size_t door = 0; door < doors.size(); ++door) {
            const std::string doorLabel = label + (door == 0 ? " door" : " door " + std::to_string(door + 1));
            if (requireWalkable(grid, name, doorLabel, doors[door], errors)) {
                anchors.push_back({doorLabel, doors[door]});
            }
            const TilePosition after = returnTileOf(doors[door]);
            if (requireWalkable(grid, name, doorLabel + " return tile", after, errors)) {
                anchors.push_back({doorLabel + " return tile", after});
            }
        }
    }
}

void checkConnected(const Grid& grid, const std::string& name, const std::vector<Anchor>& anchors,
                    std::vector<std::string>& errors) {
    if (anchors.size() < 2) {
        return;
    }
    std::vector<bool> reached(grid.walkable.size(), false);
    std::deque<TilePosition> pending;
    const auto push = [&](int x, int y) {
        if (grid.isWalkable(x, y) && !reached[static_cast<std::size_t>(y * grid.width + x)]) {
            reached[static_cast<std::size_t>(y * grid.width + x)] = true;
            pending.push_back({x, y});
        }
    };
    push(anchors.front().tile.x, anchors.front().tile.y);
    while (!pending.empty()) {
        const TilePosition tile = pending.front();
        pending.pop_front();
        push(tile.x + 1, tile.y);
        push(tile.x - 1, tile.y);
        push(tile.x, tile.y + 1);
        push(tile.x, tile.y - 1);
    }
    for (std::size_t i = 1; i < anchors.size(); ++i) {
        const TilePosition tile = anchors[i].tile;
        if (!reached[static_cast<std::size_t>(tile.y * grid.width + tile.x)]) {
            errors.push_back(name + ": " + anchors[i].label + " " + at(tile) + " is not connected with " +
                             anchors.front().label + " " + at(anchors.front().tile));
        }
    }
}

void add(std::vector<Anchor>& anchors, const std::optional<Anchor>& anchor) {
    if (anchor) {
        anchors.push_back(*anchor);
    }
}

void checkPassageExtras(const Grid& grid, const std::string& name, const char* key,
                        const std::optional<TilePosition>& door, std::vector<Anchor>& anchors,
                        std::vector<std::string>& errors) {
    if (!door) {
        return;
    }
    const std::vector<TilePosition> tiles = passageTiles(*door);
    for (std::size_t i = 1; i < tiles.size(); ++i) {
        const std::string label = std::string(key) + " " + std::to_string(i + 1);
        if (requireWalkable(grid, name, label, tiles[i], errors)) {
            anchors.push_back({label, tiles[i]});
        }
    }
}

void validateFarm(const EditorZoneData& zone, std::vector<std::string>& errors) {
    const std::string name = "Farm";
    Grid grid;
    if (!buildGrid(zone, name, grid, errors)) {
        return;
    }

    const int houses = static_cast<int>(zone.buildings.size());
    if (houses < MIN_FARM_HOUSES || houses > MAX_FARM_HOUSES) {
        errors.push_back(name + ": needs between " + std::to_string(MIN_FARM_HOUSES) + " and " +
                         std::to_string(MAX_FARM_HOUSES) + " houses (has " + std::to_string(houses) + ")");
    }
    if (zone.playerHouses && *zone.playerHouses != houses) {
        errors.push_back(name + ": player_houses (" + std::to_string(*zone.playerHouses) +
                         ") does not match the number of houses (" + std::to_string(houses) + ")");
    }

    std::vector<Anchor> anchors;
    checkBuildings(zone, grid, name, isFarmBuilding, anchors, errors);
    add(anchors, checkFixedDoor(grid, name, "door_town", zone.doorTown, true, errors));
    checkPassageExtras(grid, name, "door_town", zone.doorTown, anchors, errors);
    checkPortal(grid, name, "door_mines", PortalId::FarmToMine, zone.doorMines, anchors, errors);
    checkConnected(grid, name, anchors, errors);
}

void validateTown(const EditorZoneData& zone, std::vector<std::string>& errors) {
    const std::string name = "Town";
    Grid grid;
    if (!buildGrid(zone, name, grid, errors)) {
        return;
    }

    for (BuildingId shop: {BuildingId::PierresShop, BuildingId::ClintsShop, BuildingId::MarniesShop,
                           BuildingId::RobinsShop, BuildingId::ComunityCenter}) {
        const auto total = std::count_if(zone.buildings.begin(), zone.buildings.end(), [&](const EditorBuildingData& b) {
            return b.id == static_cast<int>(shop);
        });
        if (total != 1) {
            errors.push_back(name + ": needs exactly one " + shopName(shop) + " (has " + std::to_string(total) + ")");
        }
    }

    std::vector<Anchor> anchors;
    add(anchors, checkFixedDoor(grid, name, "door_farm", zone.doorFarm, true, errors));
    checkPassageExtras(grid, name, "door_farm", zone.doorFarm, anchors, errors);
    checkBuildings(zone, grid, name, isTownBuilding, anchors, errors);
    add(anchors, checkFixedDoor(grid, name, "door_mines", zone.doorMines, false, errors));
    checkConnected(grid, name, anchors, errors);
}

void validateLevel(const EditorZoneData& zone, std::size_t index, bool last, std::vector<std::string>& errors) {
    const std::string name = "Level " + std::to_string(index + 1);
    Grid grid;
    if (!buildGrid(zone, name, grid, errors)) {
        return;
    }

    std::vector<Anchor> anchors;
    const auto exit = checkPortal(grid, name, "door_exit", PortalId::MineToFarm, zone.doorExit, anchors, errors);
    if (last) {
        if (zone.doorNextLevel) {
            errors.push_back(name + ": the last level cannot have a ladder to the next level");
        }
    } else {
        const auto next = checkPortal(grid, name, "door_next_level", PortalId::ToNextLevel, zone.doorNextLevel, anchors,
                                      errors);
        if (exit && next && exit->tile.x == next->tile.x && exit->tile.y == next->tile.y) {
            errors.push_back(name + ": door_exit and door_next_level cannot be the same tile");
        }
    }
    checkConnected(grid, name, anchors, errors);
}

}

std::vector<TilePosition> passageTiles(const TilePosition& door) {
    std::vector<TilePosition> tiles;
    tiles.reserve(PASSAGE_HEIGHT);
    for (int i = 0; i < PASSAGE_HEIGHT; ++i) {
        tiles.push_back({door.x, door.y - i});
    }
    return tiles;
}

bool isFarmBuilding(int id) {
    return id >= static_cast<int>(BuildingId::FarmHouse) && id <= static_cast<int>(BuildingId::BrickCabin);
}

bool isTownBuilding(int id) {
    return id >= static_cast<int>(BuildingId::MarniesShop) && id <= static_cast<int>(BuildingId::ComunityCenter);
}

std::vector<std::string> validateMap(const MapData& map) {
    std::vector<std::string> errors;
    validateFarm(map.farm, errors);
    validateTown(map.town, errors);

    if (static_cast<int>(map.mineLevels.size()) != MINE_LEVEL_COUNT) {
        errors.push_back("Mine: needs exactly " + std::to_string(MINE_LEVEL_COUNT) + " levels (has " +
                         std::to_string(map.mineLevels.size()) + ")");
    }
    for (std::size_t i = 0; i < map.mineLevels.size(); ++i) {
        validateLevel(map.mineLevels[i], i, i + 1 == map.mineLevels.size(), errors);
    }
    return errors;
}

std::string joinErrors(const std::vector<std::string>& errors) {
    std::string joined;
    for (const std::string& error: errors) {
        joined += (joined.empty() ? "" : "; ") + error;
    }
    return joined;
}

}
