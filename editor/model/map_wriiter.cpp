#include "map_wriiter.h"

#include <fstream>
#include <ostream>
#include <vector>

namespace {
    std::vector<bool> blockedTiles(const ZoneMap& zone) {
        std::vector<bool> blocked(static_cast<std::size_t>(zone.getWidth() * zone.getHeight()), false);
        for (const PlacedElement& placed: zone.getElements()) {
            if (placed.element.kind == ElementKind::Portal) {
                continue;
            }
            const TileArea& footprint = ElementCatalog::infoOf(placed.element).footprint;
            for (int y = placed.y + footprint.y; y < placed.y + footprint.y + footprint.height; ++y) {
                for (int x = placed.x + footprint.x; x < placed.x + footprint.x + footprint.width; ++x) {
                    if (zone.inBounds(x, y)) {
                        blocked[static_cast<std::size_t>(y * zone.getWidth() + x)] = true;
                    }
                }
            }
        }
        return blocked;
    }

    void writeBackground(std::ostream& out, const ZoneMap& zone) {
        const std::vector<bool> blocked = blockedTiles(zone);
        out << "# cada tile: [x, y, texture_id, state]\n";
        out << "background = [\n";
        for (int y = 0; y < zone.getHeight(); ++y) {
            for (int x = 0; x < zone.getWidth(); ++x) {
                const bool walkable = !blocked[static_cast<std::size_t>(y * zone.getWidth() + x)];
                out << "  [" << x << ", " << y << ", " << static_cast<int>(zone.getFloor(x, y)) << ", \""
                    << (walkable ? "walkable" : "unwalkable") << "\"],\n";
            }
        }
        out << "]\n";
    }

    void writeBuildings(std::ostream& out, const ZoneMap& zone) {
        out << "# cada edificio: [x, y, building_id, door_x, door_y]\n";
        out << "buildings = [\n";
        for (const PlacedElement& placed: zone.getElements()) {
            if (placed.element.kind != ElementKind::Building) {
                continue;
            }
            const ElementInfo& info = ElementCatalog::infoOf(placed.element);
            out << "  [" << placed.x << ", " << placed.y << ", " << placed.element.id << ", "
                << placed.x + info.tilesWidth / 2 << ", " << placed.y + info.tilesHeight << "],\n";
        }
        out << "]\n";
    }

    void writeDecor(std::ostream& out, const ZoneMap& zone) {
        out << "# cada decoracion: [x, y, decor_id]\n";
        out << "decor = [\n";
        for (const PlacedElement& placed: zone.getElements()) {
            if (placed.element.kind == ElementKind::Decor) {
                out << "  [" << placed.x << ", " << placed.y << ", " << placed.element.id << "],\n";
            }
        }
        out << "]\n";
    }

    int countBuildings(const ZoneMap& zone) {
        int count = 0;
        for (const PlacedElement& placed: zone.getElements()) {
            if (placed.element.kind == ElementKind::Building) {
                ++count;
            }
        }
        return count;
    }
}

//el tile del medio de abajo
void writeDoor(std::ostream& out, const std::string& key, const ZoneMap& zone, PortalId portal) {
    for (const PlacedElement& placed: zone.getElements()) {
        if (placed.element == portalElement(portal)) {
            const ElementInfo& info = ElementCatalog::infoOf(placed.element);
            out << key << " = [" << placed.x + info.tilesWidth / 2 << ", " << placed.y + info.tilesHeight - 1 << "]\n";
            return;
        }
    }
}

void writeSize(std::ostream& out, const ZoneMap& zone) {
    out << "width = " << zone.getWidth() << "\n";
    out << "height = " << zone.getHeight() << "\n";
}

bool writeMap(const WorldMap& map, const std::string& path) {
    std::ofstream out(path);
    if (!out) {
        return false;
    }

    out << "[farm]\n";
    writeSize(out, map.farm);
    out << "player_houses = " << countBuildings(map.farm) << "\n";
    writeDoor(out, "door_mines", map.farm, PortalId::FarmToMine);
    writeBackground(out, map.farm);
    writeBuildings(out, map.farm);
    writeDecor(out, map.farm);

    out << "\n[town]\n";
    writeSize(out, map.town);
    writeBackground(out, map.town);
    writeBuildings(out, map.town);
    writeDecor(out, map.town);

    for (const ZoneMap& level: map.mineLevels) {
        out << "\n[[mine_levels]]\n";
        writeSize(out, level);
        writeDoor(out, "door_exit", level, PortalId::MineToFarm);
        writeDoor(out, "door_next_level", level, PortalId::ToNextLevel);
        writeBackground(out, level);
        writeDecor(out, level);
    }
    return static_cast<bool>(out);
}