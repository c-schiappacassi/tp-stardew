#include "map_wriiter.h"

#include <optional>
#include <vector>

#include "common/map/map_shapes.h"
#include "common/map/map_writer.h"

namespace {

std::vector<bool> blockedTiles(const ZoneMap& zone) {
    std::vector<bool> blocked(static_cast<std::size_t>(zone.getWidth() * zone.getHeight()), false);
    for (const PlacedElement& placed: zone.getElements()) {
        for (const TileOffset& tile: ElementCatalog::infoOf(placed.element).blocked) {
            const int x = placed.x + tile.x;
            const int y = placed.y + tile.y;
            if (zone.inBounds(x, y)) {
                blocked[static_cast<std::size_t>(y * zone.getWidth() + x)] = true;
            }
        }
    }
    return blocked;
}

std::optional<TilePosition> portalDoor(const ZoneMap& zone, PortalId portal) {
    for (const PlacedElement& placed: zone.getElements()) {
        if (placed.element == portalElement(portal)) {
            return map_rules::doorOf(map_rules::shapeOf(portal), placed.x, placed.y);
        }
    }
    return std::nullopt;
}

EditorZoneData toZoneData(const ZoneMap& zone) {
    EditorZoneData data{};
    data.width = zone.getWidth();
    data.height = zone.getHeight();

    const std::vector<bool> walkable = bakedWalkability(zone);
    data.background.reserve(walkable.size());
    for (int y = 0; y < data.height; ++y) {
        for (int x = 0; x < data.width; ++x) {
            data.background.push_back({x, y, static_cast<int>(zone.getFloor(x, y)),
                                       walkable[static_cast<std::size_t>(y * data.width + x)]});
        }
    }

    for (const PlacedElement& placed: zone.getElements()) {
        switch (placed.element.kind) {
            case ElementKind::Building: {
                const map_rules::Shape& shape = map_rules::shapeOf(static_cast<BuildingId>(placed.element.id));
                data.buildings.push_back({placed.x, placed.y, placed.element.id,
                                          map_rules::doorOf(shape, placed.x, placed.y)});
                break;
            }
            case ElementKind::Decor:
                data.decorations.push_back({placed.x, placed.y, placed.element.id});
                break;
            case ElementKind::Wall:
                data.frame.push_back({placed.x, placed.y, placed.element.id});
                break;
            case ElementKind::Portal:
                break;
        }
    }

    for (int y = 0; y < data.height; ++y) {
        for (int x = 0; x < data.width; ++x) {
            if (zone.isLocked(x, y)) {
                data.lockedTiles.push_back({x, y});
            }
        }
    }

    for (const auto& [key, door]: zone.getFixedDoors()) {
        const TilePosition position{door.first, door.second};
        if (key == "door_town") {
            data.doorTown = position;
        } else if (key == "door_farm") {
            data.doorFarm = position;
        } else if (key == "door_mines") {
            data.doorMines = position;
        }
    }
    return data;
}

}

std::vector<bool> bakedWalkability(const ZoneMap& zone) {
    const std::vector<bool> blocked = blockedTiles(zone);
    std::vector<bool> walkable(blocked.size(), false);
    for (int y = 0; y < zone.getHeight(); ++y) {
        for (int x = 0; x < zone.getWidth(); ++x) {
            const auto index = static_cast<std::size_t>(y * zone.getWidth() + x);
            const bool border = zone.isLocked(x, y) && zone.getFloor(x, y) == TileTextureId::Grass;
            walkable[index] = FloorCatalog::infoOf(zone.getFloor(x, y)).walkable && !blocked[index] && !border;
        }
    }
    return walkable;
}

MapData toMapData(const WorldMap& map) {
    MapData data{};

    data.farm = toZoneData(map.farm);
    data.farm.playerHouses = static_cast<int>(data.farm.buildings.size());
    data.farm.doorMines = portalDoor(map.farm, PortalId::FarmToMine);

    data.town = toZoneData(map.town);

    data.mineLevels.reserve(map.mineLevels.size());
    for (const ZoneMap& level: map.mineLevels) {
        EditorZoneData zone = toZoneData(level);
        zone.doorExit = portalDoor(level, PortalId::MineToFarm);
        zone.doorNextLevel = portalDoor(level, PortalId::ToNextLevel);
        data.mineLevels.push_back(std::move(zone));
    }
    return data;
}

bool writeMap(const WorldMap& map, const std::string& path) { return MapWriter().save(toMapData(map), path); }
