#include "map_reader.h"

#include <toml++/toml.hpp>

namespace {

int number(const toml::node& node) { return static_cast<int>(node.value_or<int64_t>(0)); }

void readDoor(const toml::table& table, const char* key, ZoneMap& zone, PortalId portal) {
    const toml::array* door = table[key].as_array();
    if (!door) {
        return;
    }
    const ElementInfo& info = ElementCatalog::infoOf(portalElement(portal));
    zone.place(portalElement(portal), number((*door)[0]) - info.tilesWidth / 2,
               number((*door)[1]) - info.tilesHeight + 1);
}

ZoneMap readZone(const toml::table& table) {
    ZoneMap zone(number(*table.get("width")), number(*table.get("height")), TileTextureId::Grass);

    //tile -> [x, y, texture_id, state]
    for (const toml::node& node: *table["background"].as_array()) {
        const toml::array& tile = *node.as_array();
        zone.setFloor(number(tile[0]), number(tile[1]), static_cast<TileTextureId>(number(tile[2])));
    }
    //tiles fijos (el mapa base, cliff, camino de farm a town y de town a farm...) -> [x, y]
    if (const toml::array* locked = table["locked"].as_array()) {
        for (const toml::node& node: *locked) {
            const toml::array& tile = *node.as_array();
            zone.lock(number(tile[0]), number(tile[1]));
        }
    }
    //building -> [x, y, building_id, door_x, door_y]
    if (const toml::array* buildings = table["buildings"].as_array()) {
        for (const toml::node& node: *buildings) {
            const toml::array& building = *node.as_array();
            zone.place(buildingElement(static_cast<BuildingId>(number(building[2]))), number(building[0]),
                       number(building[1]));
        }
    }
    //decor -> [x, y, decor_id]
    for (const toml::node& node: *table["decor"].as_array()) {
        const toml::array& item = *node.as_array();
        zone.place(decorElement(static_cast<DecorId>(number(item[2]))), number(item[0]), number(item[1]));
    }
    return zone;
}

}

WorldMap readMap(const std::string& path) {
    const toml::table file = toml::parse_file(path);
    WorldMap map;

    const toml::table& farm = *file["farm"].as_table();
    map.farm = readZone(farm);
    readDoor(farm, "door_mines", map.farm, PortalId::FarmToMine);

    const toml::table& town = *file["town"].as_table();
    map.town = readZone(town);

    const toml::array& levels = *file["mine_levels"].as_array();
    for (std::size_t i = 0; i < map.mineLevels.size(); ++i) {
        const toml::table& level = *levels[i].as_table();
        map.mineLevels[i] = readZone(level);
        readDoor(level, "door_exit", map.mineLevels[i], PortalId::MineToFarm);
        readDoor(level, "door_next_level", map.mineLevels[i], PortalId::ToNextLevel);
    }
    return map;
}
