#include "map_converter.h"

#include "server_constants.h"

#include <limits>
#include <stdexcept>
#include <utility>

#include "common/map/map_rules.h"
#include "dungeon/dungeon_config.h"
#include "entities/entity_dimensions.h"

static_assert(DUNGEON_ROOM_COUNT == map_rules::MINE_LEVEL_COUNT,
    SERVER_MAP_LEVEL_COUNT_ASSERTION);

ZoneMap MapConverter::convertTerrain(const EditorZoneData& zone, const std::string& name) {
    if (zone.width <= 0 || zone.height <= 0 ||
        static_cast<int64_t>(zone.width) * entity_dimensions::TILE_SIZE > UINT16_MAX ||
        static_cast<int64_t>(zone.height) * entity_dimensions::TILE_SIZE > UINT16_MAX) {
        throw std::invalid_argument(server_constants::INVALID_DIMENSIONS_FOR_PREFIX + name);
    }

    std::vector<std::vector<bool>> walkable(static_cast<std::size_t>(zone.height), std::vector<bool>(static_cast<std::size_t>(zone.width), false));
    std::vector<std::vector<bool>> seen(static_cast<std::size_t>(zone.height), std::vector<bool>(static_cast<std::size_t>(zone.width), false));

    for (const FloorTile& tile: zone.background) {
        if (tile.x < 0 || tile.y < 0 || tile.x >= zone.width || tile.y >= zone.height ||
            seen[tile.y][tile.x]) {
            throw std::invalid_argument(server_constants::INVALID_OR_DUPLICATE_FLOOR_TILE_IN_PREFIX + name);
        }
        seen[tile.y][tile.x] = true;
        walkable[tile.y][tile.x] = tile.walkable;
    }

    for (int y = 0; y < zone.height; ++y) {
        for (int x = 0; x < zone.width; ++x) {
            if (!seen[y][x]) {
                throw std::invalid_argument(server_constants::MISSING_FLOOR_TILE_IN_PREFIX + name);
            }
        }
    }

    return ZoneMap(walkable, entity_dimensions::TILE_SIZE);
}

Position MapConverter::convertPosition(const ZoneMap& terrain, const TilePosition& tile, const std::string& field) {
    const auto position = terrain.baseAt({tile.y, tile.x});
    if (!position || !terrain.isWalkableBase(*position)) {
        throw std::invalid_argument(server_constants::INVALID_OR_BLOCKED_POSITION_PREFIX + field);
    }
    return *position;
}

Position MapConverter::findSpawn(const ZoneMap& terrain) {
    const auto positions = terrain.walkableBasePositions();
    if (positions.empty()) {
        throw std::invalid_argument(server_constants::MAP_DOES_NOT_CONTAIN_ANY_WALKABLE_SPAWN_TILE);
    }
    return positions.front();
}

std::vector<Position> MapConverter::extraDoors(const ZoneMap& terrain, const EditorBuildingData& building, const std::string& field) {
    return extraDoorTiles(terrain, map_rules::doorsOf(map_rules::shapeOf(static_cast<BuildingId>(building.id)), building.x, building.y), field);
}

std::vector<Position> MapConverter::extraDoorTiles(const ZoneMap& terrain, const std::vector<TilePosition>& doors, const std::string& field) {
    std::vector<Position> extra;
    for (std::size_t i = 1; i < doors.size(); ++i) {
        extra.push_back(convertPosition(terrain, doors[i], field + server_constants::FIELD_DOOR_PREFIX + std::to_string(i) + server_constants::CLOSE_BRACKET));
    }
    return extra;
}

FarmLayout MapConverter::convertFarm(const EditorZoneData& zone) {
    FarmLayout farm;
    farm.terrain = convertTerrain(zone, server_constants::FARM);

    if (zone.buildings.empty() || zone.buildings.size() > 4) {
        throw std::invalid_argument(server_constants::A_PLAYABLE_FARM_MUST_CONTAIN_BETWEEN_1_AND_4_HOUSES);
    }
    if (zone.playerHouses && (*zone.playerHouses < 1 || static_cast<std::size_t>(*zone.playerHouses) != zone.buildings.size())) {
        throw std::invalid_argument(server_constants::FARM_PLAYER_HOUSES_DOES_NOT_MATCH_FARM_BUILDINGS);
    }

    farm.playerHouses = static_cast<uint8_t>(zone.buildings.size());
    farm.interiors.reserve(zone.buildings.size());

    for (std::size_t index = 0; index < zone.buildings.size(); ++index) {
        const EditorBuildingData& building = zone.buildings[index];
        if (building.id < static_cast<int>(BuildingId::FarmHouse) ||
            building.id > static_cast<int>(BuildingId::BrickCabin)) {
            throw std::invalid_argument(server_constants::UNSUPPORTED_BUILDING_IN_FARM_BUILDINGS_PREFIX + std::to_string(index) + server_constants::CLOSE_BRACKET);
        }

        const std::string field = server_constants::FARM_BUILDINGS_PREFIX + std::to_string(index) + server_constants::CLOSE_BRACKET;
        const Position outside = convertPosition(farm.terrain, building.door, field + server_constants::FIELD_DOOR);
        if (building.door.y >= zone.height - 1) {
            throw std::invalid_argument(server_constants::NO_RETURN_TILE_BELOW_PREFIX + field + server_constants::FIELD_DOOR);
        }
        const Position returnPosition = convertPosition(farm.terrain, map_rules::returnTileOf(building.door), field + server_constants::FIELD_RETURN);

        InteriorLayout house = makePlayerHouseInterior(static_cast<uint8_t>(index));
        house.outside.zone = Zone::Farm;
        house.outside.position = outside;
        house.extraOutside = extraDoors(farm.terrain, building, field);
        house.returnPosition = returnPosition;
        farm.interiors.push_back(std::move(house));
    }

    farm.spawn = farm.interiors.front().returnPosition;
    farm.background.reserve(zone.background.size());
    for (const FloorTile& tile: zone.background) {
        if (tile.textureId < 0 || tile.textureId > UINT16_MAX) {
            throw std::invalid_argument(server_constants::INVALID_FARM_TEXTURE_ID);
        }
        farm.background.push_back({Position{tile.x, tile.y}, static_cast<uint16_t>(tile.textureId)});
    }

    if (zone.doorTown) {
        farm.townDoor = convertPosition(farm.terrain, *zone.doorTown, server_constants::FARM_DOOR_TOWN);
        farm.extraTownDoors = extraDoorTiles(farm.terrain, map_rules::passageTiles(*zone.doorTown), server_constants::FARM_DOOR_TOWN);
        // al volver del pueblo se aparece en el tile del medio de la entrada (el (63, 9) con 3 tiles de alto)
        const std::vector<TilePosition> passage = map_rules::passageTiles(*zone.doorTown);
        farm.townReturn = convertPosition(farm.terrain, passage[passage.size() / 2], server_constants::FARM_TOWN_RETURN);
    }
    if (zone.doorMines) {
        farm.mineDoor = convertPosition(farm.terrain, *zone.doorMines, server_constants::FARM_DOOR_MINES);
        farm.mineReturn = farm.mineDoor;   // al volver de la mina se aparece en la puerta de la mina
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
        default: throw std::invalid_argument(server_constants::UNSUPPORTED_TOWN_BUILDING_ID);
    }
}

TownConfig MapConverter::convertTown(const EditorZoneData& zone, std::vector<InteriorLayout>& interiors) {
    if (zone.width <= 0 || zone.height <= 0 || zone.width > UINT16_MAX || zone.height > UINT16_MAX) {
        throw std::invalid_argument(server_constants::INVALID_TOWN_DIMENSIONS);
    }

    TownConfig town;
    town.map = {static_cast<uint16_t>(zone.width), static_cast<uint16_t>(zone.height), static_cast<uint16_t>(entity_dimensions::TILE_SIZE)};

    const ZoneMap terrain = convertTerrain(zone, server_constants::TOWN);
    std::vector<std::vector<bool>> walkable(static_cast<std::size_t>(zone.height), std::vector<bool>(static_cast<std::size_t>(zone.width), false));
    for (int y = 0; y < zone.height; ++y) {
        for (int x = 0; x < zone.width; ++x) {
            const auto base = terrain.baseAt({y, x});
            walkable[y][x] = base && terrain.isWalkableBase(*base);
        }
    }
    town.walkableTiles = std::move(walkable);

    if (zone.doorFarm) {
        convertPosition(terrain, *zone.doorFarm, server_constants::TOWN_DOOR_FARM);
        town.connections.push_back({server_constants::FARM, {static_cast<uint16_t>(zone.doorFarm->x), static_cast<uint16_t>(zone.doorFarm->y)}});
        const std::vector<TilePosition> passage = map_rules::passageTiles(*zone.doorFarm);
        for (std::size_t i = 1; i < passage.size(); ++i) {
            convertPosition(terrain, passage[i], server_constants::TOWN_DOOR_FARM_PREFIX + std::to_string(i) + server_constants::CLOSE_BRACKET);
            town.extraFarmDoors.push_back({static_cast<uint16_t>(passage[i].x), static_cast<uint16_t>(passage[i].y)});
        }
    }
    if (zone.doorMines) {
        convertPosition(terrain, *zone.doorMines, server_constants::TOWN_DOOR_MINES);
        town.connections.push_back({server_constants::DUNGEON, {static_cast<uint16_t>(zone.doorMines->x), static_cast<uint16_t>(zone.doorMines->y)}});
    }

    std::vector<bool> usedInteriors(static_cast<std::size_t>(InteriorId::ComunityCenter) + 1, false);
    for (const EditorBuildingData& building: zone.buildings) {
        const InteriorId interiorId = townInteriorId(building.id);
        const std::size_t key = static_cast<std::size_t>(interiorId);
        if (usedInteriors[key]) {
            throw std::invalid_argument(server_constants::DUPLICATE_TOWN_INTERIOR_IN_TOML);
        }
        usedInteriors[key] = true;

        // Los edificios del pueblo ya vienen horneados en `walkableTiles` (el `background` del
        // TOML es la unica verdad), asi que no se le pasan rectangulos al TownMap para que
        // vuelva a bloquearlos.
        const Position door = convertPosition(terrain, building.door, server_constants::TOWN_BUILDING_DOOR);
        const Position returnPosition = convertPosition(terrain, map_rules::returnTileOf(building.door), server_constants::TOWN_BUILDING_RETURN);
        InteriorLayout room = makeTownInterior(interiorId);
        room.outside.position = door;
        room.extraOutside = extraDoors(terrain, building, server_constants::TOWN_BUILDING);
        room.returnPosition = returnPosition;
        interiors.push_back(std::move(room));
    }
    return town;
}

DungeonRoomLayout MapConverter::convertRoom(const EditorZoneData& zone, uint8_t level) {
    const std::string name = server_constants::MINE_LEVELS_PREFIX + std::to_string(level - 1) + server_constants::CLOSE_BRACKET;

    DungeonRoomLayout room;
    room.terrain = convertTerrain(zone, name);

    if (!zone.doorExit) {
        throw std::invalid_argument(server_constants::MISSING_PREFIX + name + server_constants::FIELD_DOOR_EXIT);
    }

    room.passages.exit = convertPosition(room.terrain, *zone.doorExit, name + server_constants::FIELD_DOOR_EXIT);
    room.passages.entry = room.passages.exit;
    const map_rules::Shape& exitShape = map_rules::shapeOf(PortalId::MineToFarm);
    const TilePosition exitOrigin = map_rules::originOf(exitShape, *zone.doorExit);
    room.passages.extraExits = extraDoorTiles(room.terrain, map_rules::doorsOf(exitShape, exitOrigin.x, exitOrigin.y), name + server_constants::FIELD_DOOR_EXIT);

    if (zone.doorNextLevel) {
        const Position next = convertPosition(room.terrain, *zone.doorNextLevel, name + server_constants::FIELD_DOOR_NEXT_LEVEL);

        if (room.terrain.sameBaseCell(room.passages.exit, next)) {
            throw std::invalid_argument(server_constants::OVERLAPPING_MINE_PASSAGES_IN_PREFIX + name);
        }
        if (!room.terrain.hasBasePath(room.passages.entry, next)) {
            throw std::invalid_argument(server_constants::DISCONNECTED_MINE_PASSAGES_IN_PREFIX + name);
        }

        room.passages.nextRoom = next;
    } else if (level < DUNGEON_ROOM_COUNT) {
        throw std::invalid_argument(server_constants::MISSING_PREFIX + name + server_constants::FIELD_DOOR_NEXT_LEVEL);
    }

    return room;
}

WorldSetup MapConverter::convert(const MapData& data, uint8_t worldId,
                                 const InteriorTemplates& interiors) const {
    auto validationData = data;
    if (!validationData.mineLevels.empty())
        validationData.mineLevels.back().doorNextLevel.reset();
    auto errors = map_rules::validateMap(validationData);
    if (!errors.empty()) {
        throw std::invalid_argument(server_constants::INVALID_MAP_PREFIX + map_rules::joinErrors(errors));
    }
    if (data.mineLevels.back().doorNextLevel) {
        validationData.mineLevels.front() = data.mineLevels.back();
        errors = map_rules::validateMap(validationData);
        if (!errors.empty())
            throw std::invalid_argument(server_constants::INVALID_LAST_MINE_TEMPLATE_PREFIX + map_rules::joinErrors(errors));
    }
    if (data.mineLevels.size() != DUNGEON_ROOM_COUNT) {
        throw std::invalid_argument(server_constants::UNEXPECTED_NUMBER_OF_MINE_LEVELS);
    }

    WorldSetup setup;
    setup.worldId = worldId;
    setup.farm = convertFarm(data.farm);
    setup.townConfig = convertTown(data.town, setup.farm.interiors);
    for (auto& room: setup.farm.interiors) {
        const auto found = interiors.find(room.location.interior_id);
        if (found != interiors.end())
            found->second.applyTo(room);
    }

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

BackgroundData MapConverter::convertVisualZone(const EditorZoneData& zone, const std::string& name) {
    BackgroundData result;
    const int tileSize = entity_dimensions::TILE_SIZE;
    result.floor.reserve(zone.background.size());
    result.decor.reserve(zone.decorations.size());
    result.frame.reserve(zone.frame.size());
    result.buildings.reserve(zone.buildings.size());

    const auto position = [&](int x, int y) -> Position {
        if (x < 0 || y < 0 || x >= zone.width || y >= zone.height) {
            throw std::invalid_argument(server_constants::VISUAL_ELEMENT_OUTSIDE_PREFIX + name);
        }
        return Position(x * tileSize, y * tileSize);
    };
    for (const FloorTile& tile: zone.background) {
        if (tile.textureId < 0 || tile.textureId > static_cast<int>(TileTextureId::DungeonInnerCornerTopLeft)) {
            throw std::invalid_argument(server_constants::UNKNOWN_FLOOR_TEXTURE_IN_PREFIX + name);
        }
        result.floor.push_back({position(tile.x, tile.y), static_cast<TileTextureId>(tile.textureId)});
    }
    for (const MapElement& decor : zone.decorations) {
        if (decor.id < 0 || decor.id > static_cast<int>(DecorId::SignPost)) {
            throw std::invalid_argument(server_constants::UNKNOWN_DECORATION_IN_PREFIX + name);
        }
        result.decor.push_back({position(decor.x, decor.y), static_cast<DecorId>(decor.id)});
    }
    for (const MapElement& piece: zone.frame) {
        if (piece.id < 0 || piece.id > static_cast<int>(FramePieceId::DungeonTopWallEndLeft)) {
            throw std::invalid_argument(server_constants::UNKNOWN_FRAME_PIECE_IN_PREFIX + name);
        }
        result.frame.push_back({position(piece.x, piece.y), static_cast<FramePieceId>(piece.id)});
    }
    for (const EditorBuildingData& building: zone.buildings) {
        if (building.id < 0 || building.id > static_cast<int>(BuildingId::ComunityCenter)) {
            throw std::invalid_argument(server_constants::UNKNOWN_BUILDING_IN_PREFIX + name);
        }
        result.buildings.push_back({position(building.x, building.y), static_cast<BuildingId>(building.id)});
    }
    return result;
}

ConstMapData MapConverter::convertVisuals(const MapData& data) const {
    ConstMapData visuals;
    visuals.farm = convertVisualZone(data.farm, server_constants::FARM);
    visuals.town = convertVisualZone(data.town, server_constants::TOWN);
    visuals.levels.reserve(data.mineLevels.size());
    for (std::size_t index = 0; index < data.mineLevels.size(); ++index) {
        visuals.levels.push_back(convertVisualZone(data.mineLevels[index], server_constants::MINE_LEVELS_PREFIX + std::to_string(index) + server_constants::CLOSE_BRACKET));
    }
    return visuals;
}
