#include "map_reader.h"

#include <stdexcept>
#include <string>
#include <vector>

#include "common/map/map_loader.h"
#include "common/map/map_shapes.h"

namespace {

std::string at(int x, int y) { return "(" + std::to_string(x) + ", " + std::to_string(y) + ")"; }

void placeBuilding(ZoneMap& zone, const EditorBuildingData& building, const std::string& zoneName,
                   std::vector<std::string>& errors) {
    if (building.id < 0 || building.id > static_cast<int>(BuildingId::ComunityCenter)) {
        errors.push_back(zoneName + ": unknown building " + std::to_string(building.id));
        return;
    }
    if (!zone.place(buildingElement(static_cast<BuildingId>(building.id)), building.x, building.y)) {
        errors.push_back(zoneName + ": the building " + std::to_string(building.id) + " at " +
                         at(building.x, building.y) + " can't be placed");
    }
}

void placePortal(ZoneMap& zone, const std::optional<TilePosition>& door, PortalId portal, const std::string& zoneName,
                 std::vector<std::string>& errors) {
    if (!door) {
        return;
    }
    const TilePosition origin = map_rules::originOf(map_rules::shapeOf(portal), *door);
    if (!zone.place(portalElement(portal), origin.x, origin.y)) {
        errors.push_back(zoneName + ": the portal with its door at " + at(door->x, door->y) + " can't be placed");
    }
}

ZoneMap toZone(const EditorZoneData& data, const std::string& zoneName, std::vector<std::string>& errors) {
    ZoneMap zone(data.width, data.height, TileTextureId::Grass);

    for (const FloorTile& tile: data.background) {
        zone.setFloor(tile.x, tile.y, static_cast<TileTextureId>(tile.textureId));
    }
    for (const EditorBuildingData& building: data.buildings) {
        placeBuilding(zone, building, zoneName, errors);
    }
    for (const MapElement& item: data.decorations) {
        zone.place(decorElement(static_cast<DecorId>(item.id)), item.x, item.y);
    }
    for (const MapElement& piece: data.frame) {
        zone.place(wallElement(static_cast<FramePieceId>(piece.id)), piece.x, piece.y);
    }
    for (const TilePosition& tile: data.lockedTiles) {
        zone.lock(tile.x, tile.y);
    }
    if (data.doorTown) {
        zone.setFixedDoor("door_town", data.doorTown->x, data.doorTown->y);
    }
    if (data.doorFarm) {
        zone.setFixedDoor("door_farm", data.doorFarm->x, data.doorFarm->y);
    }
    return zone;
}

}

WorldMap toWorldMap(const MapData& data) {
    std::vector<std::string> errors;
    WorldMap map;

    map.farm = toZone(data.farm, "Farm", errors);
    placePortal(map.farm, data.farm.doorMines, PortalId::FarmToMine, "Farm", errors);

    map.town = toZone(data.town, "Town", errors);
    if (data.town.doorMines) {
        map.town.setFixedDoor("door_mines", data.town.doorMines->x, data.town.doorMines->y);
    }

    map.mineLevels.clear();
    for (std::size_t i = 0; i < data.mineLevels.size(); ++i) {
        const std::string name = "Level " + std::to_string(i + 1);
        ZoneMap level = toZone(data.mineLevels[i], name, errors);
        placePortal(level, data.mineLevels[i].doorExit, PortalId::MineToFarm, name, errors);
        placePortal(level, data.mineLevels[i].doorNextLevel, PortalId::ToNextLevel, name, errors);
        map.mineLevels.push_back(std::move(level));
    }

    if (!errors.empty()) {
        std::string message = "The map has elements that can't be placed:";
        for (const std::string& error: errors) {
            message += "\n- " + error;
        }
        throw std::runtime_error(message);
    }
    return map;
}

WorldMap readMap(const std::string& path) { return toWorldMap(MapLoader().load(path)); }
