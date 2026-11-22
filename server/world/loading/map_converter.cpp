#include "map_converter.h"

#include <limits>
#include <stdexcept>
#include <utility>

#include "dungeon/dungeon_config.h"
#include "entities/entity_dimensions.h"

ZoneMap MapConverter::convertTerrain(const EditorZoneData& zone, const std::string& name) {
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

FarmLayout MapConverter::convertFarm(const EditorZoneData& zone) {
    FarmLayout farm;
    farm.terrain = convertTerrain(zone, "farm");

    if (zone.buildings.empty() || zone.buildings.size() > 4) {
        throw std::invalid_argument("A playable farm must contain between 1 and 4 houses");
    }
    if (zone.playerHouses &&
        (*zone.playerHouses < 1 ||
         static_cast<std::size_t>(*zone.playerHouses) != zone.buildings.size())) {
        throw std::invalid_argument("farm.player_houses does not match farm.buildings");
    }

    farm.playerHouses = static_cast<uint8_t>(zone.buildings.size());
    farm.interiors.reserve(zone.buildings.size());

    for (std::size_t index = 0; index < zone.buildings.size(); ++index) {
        const EditorBuildingData& building = zone.buildings[index];
        if (building.id < static_cast<int>(BuildingId::FarmHouse) ||
            building.id > static_cast<int>(BuildingId::BrickCabin)) {
            throw std::invalid_argument("Unsupported building in farm.buildings[" +
                                        std::to_string(index) + "]");
        }

        const std::string field = "farm.buildings[" + std::to_string(index) + "]";
        const Position outside = convertPosition(farm.terrain, building.door, field + ".door");
        if (building.door.y >= zone.height - 1) {
            throw std::invalid_argument("No return tile below " + field + ".door");
        }
        const Position returnPosition = convertPosition(
            farm.terrain, {building.door.x, building.door.y + 1}, field + ".return");

        InteriorLayout house = makePlayerHouseInterior(static_cast<uint8_t>(index));
        house.outside.zone = Zone::Farm;
        house.outside.position = outside;
        house.returnPosition = returnPosition;
        farm.interiors.push_back(std::move(house));
    }

    farm.spawn = farm.interiors.front().returnPosition;
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

InteriorId MapConverter::townInteriorId(int buildingId) {
    switch (static_cast<BuildingId>(buildingId)) {
        case BuildingId::MarniesShop: return InteriorId::MarniesShop;
        case BuildingId::PierresShop: return InteriorId::PierresShop;
        case BuildingId::RobinsShop: return InteriorId::RobinsShop;
        case BuildingId::ClintsShop: return InteriorId::ClintsShop;
        case BuildingId::ComunityCenter: return InteriorId::ComunityCenter;
        default: throw std::invalid_argument("Unsupported town building ID");
    }
}

TownBuildingConfig MapConverter::convertTownBuilding(const EditorBuildingData& building) {
    TownBuildingConfig config;
    switch (static_cast<BuildingId>(building.id)) {
        case BuildingId::MarniesShop: config.id = "marnies_shop"; config.width = 8; config.height = 7; break;
        case BuildingId::PierresShop: config.id = "pierres_shop"; config.width = 11; config.height = 10; break;
        case BuildingId::RobinsShop: config.id = "robins_shop"; config.width = 10; config.height = 13; break;
        case BuildingId::ClintsShop: config.id = "clints_shop"; config.width = 9; config.height = 8; break;
        case BuildingId::ComunityCenter: config.id = "community_center"; config.width = 12; config.height = 10; break;
        default: throw std::invalid_argument("Unsupported town building ID");
    }
    if (building.x < 0 || building.y < 0 ||
        building.x > UINT16_MAX || building.y > UINT16_MAX) {
        throw std::invalid_argument("Invalid town building coordinates");
    }
    config.position = {static_cast<uint16_t>(building.x), static_cast<uint16_t>(building.y)};
    config.door.push_back({static_cast<uint16_t>(building.door.x),
                           static_cast<uint16_t>(building.door.y)});
    return config;
}

TownConfig MapConverter::convertTown(const EditorZoneData& zone,
                                      std::vector<InteriorLayout>& interiors) {
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

    std::vector<bool> usedInteriors(static_cast<std::size_t>(InteriorId::ComunityCenter) + 1, false);
    for (const EditorBuildingData& building : zone.buildings) {
        const InteriorId interiorId = townInteriorId(building.id);
        const std::size_t key = static_cast<std::size_t>(interiorId);
        if (usedInteriors[key]) {
            throw std::invalid_argument("Duplicate town interior in TOML");
        }
        usedInteriors[key] = true;

        TownBuildingConfig config = convertTownBuilding(building);
        if (building.door.x < 0 || building.door.y < 0 ||
            building.door.x >= zone.width || building.door.y >= zone.height ||
            building.door.x < building.x || building.door.y < building.y ||
            building.door.x >= building.x + config.width ||
            building.door.y >= building.y + config.height) {
            throw std::invalid_argument("Invalid town building door");
        }
        const Position door = convertPosition(terrain, building.door, "town.building.door");
        const Position returnPosition = convertPosition(
            terrain, {building.door.x, building.door.y + 1}, "town.building.return");
        InteriorLayout room = makeTownInterior(interiorId);
        room.outside.position = door;
        room.returnPosition = returnPosition;
        interiors.push_back(std::move(room));
        town.buildings.push_back(std::move(config));
    }
    return town;
}

DungeonRoomLayout MapConverter::convertRoom(const EditorZoneData& zone, uint8_t level) {
    const std::string name = "mine_levels[" + std::to_string(level - 1) + "]";

    DungeonRoomLayout room;
    room.terrain = convertTerrain(zone, name);

    if (!zone.doorExit) {
        throw std::invalid_argument("Missing " + name + ".door_exit");
    }

    room.passages.exit = convertPosition(room.terrain, *zone.doorExit, name + ".door_exit");
    room.passages.entry = room.passages.exit;

    if (level < DUNGEON_ROOM_COUNT) {
        if (!zone.doorNextLevel) {
            throw std::invalid_argument("Missing " + name + ".door_next_level");
        }

        const Position next = convertPosition(
                room.terrain, *zone.doorNextLevel, name + ".door_next_level");

        if (room.terrain.sameBaseCell(room.passages.exit, next)) {
            throw std::invalid_argument("Overlapping mine passages in " + name);
        }
        if (!room.terrain.hasBasePath(room.passages.entry, next)) {
            throw std::invalid_argument("Disconnected mine passages in " + name);
        }

        room.passages.nextRoom = next;
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
    setup.townConfig = convertTown(data.town, setup.farm.interiors);

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

BackgroundData MapConverter::convertVisualZone(const EditorZoneData& zone,
                                                const std::string& name) {
    BackgroundData result;
    const int tileSize = entity_dimensions::TILE_SIZE;
    result.floor.reserve(zone.background.size());
    result.decor.reserve(zone.decorations.size());
    result.buildings.reserve(zone.buildings.size());

    const auto position = [&](int x, int y) -> Position {
        if (x < 0 || y < 0 || x >= zone.width || y >= zone.height) {
            throw std::invalid_argument("Visual element outside " + name);
        }
        return {x * tileSize, y * tileSize};
    };
    for (const FloorTile& tile : zone.background) {
        if (tile.textureId < 0 || tile.textureId > static_cast<int>(TileTextureId::DungeonInnerCornerTopLeft)) {
            throw std::invalid_argument("Unknown floor texture in " + name);
        }
        result.floor.push_back({position(tile.x, tile.y), static_cast<TileTextureId>(tile.textureId)});
    }
    for (const MapElement& decor : zone.decorations) {
        if (decor.id < 0 || decor.id > static_cast<int>(DecorId::BrownArmCouch)) {
            throw std::invalid_argument("Unknown decoration in " + name);
        }
        result.decor.push_back({position(decor.x, decor.y), static_cast<DecorId>(decor.id)});
    }
    for (const EditorBuildingData& building : zone.buildings) {
        if (building.id < 0 || building.id > static_cast<int>(BuildingId::ComunityCenter)) {
            throw std::invalid_argument("Unknown building in " + name);
        }
        result.buildings.push_back({position(building.x, building.y), static_cast<BuildingId>(building.id)});
    }
    return result;
}

ConstMapData MapConverter::convertVisuals(const MapData& data) const {
    ConstMapData visuals;
    visuals.farm = convertVisualZone(data.farm, "farm");
    visuals.town = convertVisualZone(data.town, "town");
    visuals.levels.reserve(data.mineLevels.size());
    for (std::size_t index = 0; index < data.mineLevels.size(); ++index) {
        visuals.levels.push_back(convertVisualZone(
            data.mineLevels[index], "mine_levels[" + std::to_string(index) + "]"));
    }
    return visuals;
}
