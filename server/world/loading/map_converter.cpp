#include "map_converter.h"

#include <limits>
#include <stdexcept>
#include <utility>

#include "dungeon/dungeon_config.h"
#include "entities/entity_dimensions.h"

ZoneMap MapConverter::convertTerrain(const ZoneData& zone, const std::string& name) {
    if (zone.width <= 0 || zone.height <= 0 ||
        static_cast<int64_t>(zone.width) * entity_dimensions::TILE_SIZE > UINT16_MAX ||
        static_cast<int64_t>(zone.height) * entity_dimensions::TILE_SIZE > UINT16_MAX) {
        throw std::invalid_argument("Invalid dimensions for " + name);
    }

    std::vector<std::vector<bool>> walkable(
            static_cast<std::size_t>(zone.height),
            std::vector<bool>(static_cast<std::size_t>(zone.width), false));
    std::vector<std::vector<bool>> seen(
            static_cast<std::size_t>(zone.height),
            std::vector<bool>(static_cast<std::size_t>(zone.width), false));

    for (const FloorTile& tile : zone.background) {
        if (tile.x < 0 || tile.y < 0 || tile.x >= zone.width || tile.y >= zone.height ||
            seen[tile.y][tile.x]) {
            throw std::invalid_argument("Invalid or duplicate floor tile in " + name);
        }
        seen[tile.y][tile.x] = true;
        walkable[tile.y][tile.x] = tile.walkable;
    }

    for (int y = 0; y < zone.height; ++y) {
        for (int x = 0; x < zone.width; ++x) {
            if (!seen[y][x]) {
                throw std::invalid_argument("Missing floor tile in " + name);
            }
        }
    }

    return ZoneMap(walkable, entity_dimensions::TILE_SIZE);
}

Position MapConverter::convertPosition(const ZoneMap& terrain, const TilePosition& tile,
                                       const std::string& field) {
    const auto position = terrain.baseAt({tile.y, tile.x});
    if (!position || !terrain.isWalkableBase(*position)) {
        throw std::invalid_argument("Invalid or blocked position: " + field);
    }
    return *position;
}

Position MapConverter::findSpawn(const ZoneMap& terrain) {
    const auto positions = terrain.walkableBasePositions();
    if (positions.empty()) {
        throw std::invalid_argument("Map does not contain any walkable spawn tile");
    }
    return positions.front();
}

FarmLayout MapConverter::convertFarm(const ZoneData& zone) {
    FarmLayout farm;
    farm.terrain = convertTerrain(zone, "farm");
    farm.spawn = findSpawn(farm.terrain);
    if (zone.playerHouses) {
        if (*zone.playerHouses < 0 || *zone.playerHouses > 4) {
            throw std::invalid_argument("Invalid player_houses count");
        }
        farm.playerHouses = static_cast<uint8_t>(*zone.playerHouses);
    }

    farm.background.reserve(zone.background.size());
    for (const FloorTile& tile : zone.background) {
        if (tile.textureId < 0 || tile.textureId > UINT16_MAX) {
            throw std::invalid_argument("Invalid farm texture ID");
        }
        farm.background.push_back({Position{tile.x, tile.y},
                                   static_cast<uint16_t>(tile.textureId)});
    }

    if (zone.doorTown) {
        farm.townDoor = convertPosition(farm.terrain, *zone.doorTown, "farm.door_town");
        farm.townReturn = farm.spawn;
    }
    if (zone.doorMines) {
        farm.mineDoor = convertPosition(farm.terrain, *zone.doorMines, "farm.door_mines");
        farm.mineReturn = farm.spawn;
    }
    return farm;
}

TownConfig MapConverter::convertTown(const ZoneData& zone) {
    if (zone.width <= 0 || zone.height <= 0 || zone.width > UINT16_MAX ||
        zone.height > UINT16_MAX) {
        throw std::invalid_argument("Invalid town dimensions");
    }

    TownConfig town;
    town.map = {static_cast<uint16_t>(zone.width), static_cast<uint16_t>(zone.height),
                static_cast<uint16_t>(entity_dimensions::TILE_SIZE)};

    const ZoneMap terrain = convertTerrain(zone, "town");
    std::vector<std::vector<bool>> walkable(
        static_cast<std::size_t>(zone.height),
        std::vector<bool>(static_cast<std::size_t>(zone.width), false));
    for (int y = 0; y < zone.height; ++y) {
        for (int x = 0; x < zone.width; ++x) {
            const auto base = terrain.baseAt({y, x});
            walkable[y][x] = base && terrain.isWalkableBase(*base);
        }
    }
    town.walkableTiles = std::move(walkable);

    if (zone.doorFarm) {
        convertPosition(terrain, *zone.doorFarm, "town.door_farm");
        town.connections.push_back({"farm", {static_cast<uint16_t>(zone.doorFarm->x),
                                               static_cast<uint16_t>(zone.doorFarm->y)}});
    }
    if (zone.doorMines) {
        convertPosition(terrain, *zone.doorMines, "town.door_mines");
        town.connections.push_back({"dungeon", {static_cast<uint16_t>(zone.doorMines->x),
                                                  static_cast<uint16_t>(zone.doorMines->y)}});
    }

    return town;
}

DungeonRoomLayout MapConverter::convertRoom(const ZoneData& zone, uint8_t level) {
    DungeonRoomLayout room;
    room.terrain = convertTerrain(zone, "mine level " + std::to_string(level));
    if (!zone.doorExit) {
        throw std::invalid_argument("Missing door_exit for mine level " + std::to_string(level));
    }
    room.passages.exit = convertPosition(room.terrain, *zone.doorExit, "mine.door_exit");
    // exit?
    room.passages.entry = room.passages.exit;
    if (level < DUNGEON_ROOM_COUNT) {
        if (!zone.doorNextLevel) {
            throw std::invalid_argument("Missing door_next_level for mine level " +
                                        std::to_string(level));
        }
        room.passages.nextRoom = convertPosition(room.terrain, *zone.doorNextLevel,
                                                 "mine.door_next_level");
    } else if (zone.doorNextLevel) {
        throw std::invalid_argument("Last mine level cannot have a next-level door");
    }
    return room;
}

GameSetup MapConverter::convert(const MapData& data, uint8_t worldId) const {
    if (data.mineLevels.size() != DUNGEON_ROOM_COUNT) {
        throw std::invalid_argument("Unexpected number of mine levels");
    }

    GameSetup setup;
    setup.worldId = worldId;
    setup.farm = convertFarm(data.farm);
    setup.townConfig = convertTown(data.town);

    for (std::size_t index = 0; index < data.mineLevels.size(); ++index) {
        const uint8_t level = static_cast<uint8_t>(index + 1);
        setup.layout.emplace(level, convertRoom(data.mineLevels[index], level));
    }

    if (setup.farm.mineDoor) {
        setup.entrance.emplace();
        setup.entrance->zone = Zone::Farm;
        setup.entrance->terrain = setup.farm.terrain;
        setup.entrance->position = *setup.farm.mineDoor;
        setup.entrance->returnPosition = setup.farm.mineReturn.value_or(setup.farm.spawn);
    }
    return setup;
}
