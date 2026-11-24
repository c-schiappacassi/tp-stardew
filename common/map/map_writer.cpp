#include "common/map/map_writer.h"

#include <fstream>
#include <optional>

namespace {

void writeDoor(std::ostream& out, const char* key, const std::optional<TilePosition>& door) {
    if (door) {
        out << key << " = [" << door->x << ", " << door->y << "]\n";
    }
}

void writeBackground(std::ostream& out, const EditorZoneData& zone) {
    out << "background = [\n";
    for (const FloorTile& tile: zone.background) {
        out << "  [" << tile.x << ", " << tile.y << ", " << tile.textureId << ", \""
            << (tile.walkable ? "walkable" : "unwalkable") << "\"],\n";
    }
    out << "]\n";
}

void writeBuildings(std::ostream& out, const EditorZoneData& zone) {
    out << "buildings = [\n";
    for (const EditorBuildingData& building: zone.buildings) {
        out << "  [" << building.x << ", " << building.y << ", " << building.id << ", " << building.door.x << ", "
            << building.door.y << "],\n";
    }
    out << "]\n";
}

void writeElements(std::ostream& out, const char* key, const std::vector<MapElement>& elements) {
    out << key << " = [\n";
    for (const MapElement& element: elements) {
        out << "  [" << element.x << ", " << element.y << ", " << element.id << "],\n";
    }
    out << "]\n";
}

void writeLocked(std::ostream& out, const EditorZoneData& zone) {
    out << "locked = [\n";
    for (const TilePosition& tile: zone.lockedTiles) {
        out << "  [" << tile.x << ", " << tile.y << "],\n";
    }
    out << "]\n";
}

}

void MapWriter::writeZone(std::ostream& out, const EditorZoneData& zone, bool withBuildings) {
    out << "width = " << zone.width << "\n";
    out << "height = " << zone.height << "\n";
    if (zone.playerHouses) {
        out << "player_houses = " << *zone.playerHouses << "\n";
    }
    writeDoor(out, "door_town", zone.doorTown);
    writeDoor(out, "door_farm", zone.doorFarm);
    writeDoor(out, "door_mines", zone.doorMines);
    writeDoor(out, "door_exit", zone.doorExit);
    writeDoor(out, "door_next_level", zone.doorNextLevel);
    writeBackground(out, zone);
    if (withBuildings) {
        writeBuildings(out, zone);
    }
    writeElements(out, "decor", zone.decorations);
    writeElements(out, "frame", zone.frame);
    writeLocked(out, zone);
}

void MapWriter::write(std::ostream& out, const MapData& map) const {
    out << "[farm]\n";
    writeZone(out, map.farm, true);
    out << "\n[town]\n";
    writeZone(out, map.town, true);
    for (const EditorZoneData& level: map.mineLevels) {
        out << "\n[[mine_levels]]\n";
        writeZone(out, level, false);
    }
}

bool MapWriter::save(const MapData& map, const std::filesystem::path& path) const {
    std::ofstream out(path);
    if (!out) {
        return false;
    }
    write(out, map);
    return static_cast<bool>(out);
}
