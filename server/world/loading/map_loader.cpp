#include "map_loader.h"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

#include <toml++/toml.hpp>

std::runtime_error MapLoader::invalidField(const std::string& field) {
    return std::runtime_error("Invalid map field: " + field);
}

const toml::table& MapLoader::requireTable(const toml::node* node, const std::string& field) {
    const toml::table* table = node ? node->as_table() : nullptr;
    if (!table) {
        throw invalidField(field);
    }
    return *table;
}

const toml::array& MapLoader::requireArray(const toml::node* node, const std::string& field) {
    const toml::array* array = node ? node->as_array() : nullptr;
    if (!array) {
        throw invalidField(field);
    }
    return *array;
}

int MapLoader::requireInt(const toml::node* node, const std::string& field) {
    const std::optional<std::int64_t> value = node ? node->value<std::int64_t>() : std::nullopt;
    if (!value || *value < std::numeric_limits<int>::min() || *value > std::numeric_limits<int>::max()) {
        throw invalidField(field);
    }
    return static_cast<int>(*value);
}

const toml::node* MapLoader::at(const toml::array& array, std::size_t index) {
    return array.get(index);
}

void MapLoader::requireSize(const toml::array& array, std::size_t expected, const std::string& field) {
    if (array.size() != expected) {
        throw invalidField(field);
    }
}

TilePosition MapLoader::readPosition(const toml::array& array, const std::string& field) {
    requireSize(array, 2, field);
    return {requireInt(at(array, 0), field), requireInt(at(array, 1), field)};
}

std::optional<TilePosition> MapLoader::readDoor(const toml::table& table, const char* key, const std::string& zone) {
    const toml::node* node = table.get(key);
    if (!node) {
        return std::nullopt;
    }
    return readPosition(requireArray(node, zone + "." + key), zone + "." + key);
}

std::vector<FloorTile> MapLoader::readBackground(const toml::table& table, const std::string& zone) {
    const std::string field = zone + ".background";
    const toml::array& entries = requireArray(table.get("background"), field);
    std::vector<FloorTile> result;
    result.reserve(entries.size());
    for (const toml::node& node: entries) {
        const toml::array& entry = requireArray(&node, field);
        requireSize(entry, 4, field);
        const std::optional<std::string> state = at(entry, 3)->value<std::string>();
        if (!state || (*state != "walkable" && *state != "unwalkable")) {
            throw invalidField(field);
        }
        result.push_back({requireInt(at(entry, 0), field), requireInt(at(entry, 1), field),
                          requireInt(at(entry, 2), field), *state == "walkable"});
    }
    return result;
}

std::vector<MapElement> MapLoader::readElements(const toml::table& table, const char* key, const std::string& zone) {
    const std::string field = zone + "." + key;
    const toml::node* node = table.get(key);
    if (!node) {
        return {};
    }
    const toml::array& entries = requireArray(node, field);
    std::vector<MapElement> result;
    result.reserve(entries.size());
    for (const toml::node& item: entries) {
        const toml::array& entry = requireArray(&item, field);
        requireSize(entry, 3, field);
        result.push_back({requireInt(at(entry, 0), field), requireInt(at(entry, 1), field),
                          requireInt(at(entry, 2), field)});
    }
    return result;
}

std::vector<EditorBuildingData> MapLoader::readBuildings(const toml::table& table, const std::string& zone) {
    const std::string field = zone + ".buildings";
    const toml::node* node = table.get("buildings");
    if (!node) {
        return {};
    }
    const toml::array& entries = requireArray(node, field);
    std::vector<EditorBuildingData> result;
    result.reserve(entries.size());
    for (const toml::node& item: entries) {
        const toml::array& entry = requireArray(&item, field);
        requireSize(entry, 5, field);
        result.push_back({requireInt(at(entry, 0), field), requireInt(at(entry, 1), field),
                          requireInt(at(entry, 2), field),
                          {requireInt(at(entry, 3), field), requireInt(at(entry, 4), field)}});
    }
    return result;
}

std::vector<TilePosition> MapLoader::readLockedTiles(const toml::table& table, const std::string& zone) {
    const std::string field = zone + ".locked";
    const toml::node* node = table.get("locked");
    if (!node) {
        return {};
    }
    const toml::array& entries = requireArray(node, field);
    std::vector<TilePosition> result;
    result.reserve(entries.size());
    for (const toml::node& item: entries) {
        result.push_back(readPosition(requireArray(&item, field), field));
    }
    return result;
}

EditorZoneData MapLoader::readZone(const toml::table& table, const std::string& name) {
    EditorZoneData zone{};
    zone.width = requireInt(table.get("width"), name + ".width");
    zone.height = requireInt(table.get("height"), name + ".height");
    if (zone.width <= 0 || zone.height <= 0) {
        throw invalidField(name + ".dimensions");
    }
    zone.background = readBackground(table, name);
    zone.buildings = readBuildings(table, name);
    zone.decorations = readElements(table, "decor", name);
    zone.frame = readElements(table, "frame", name);
    zone.lockedTiles = readLockedTiles(table, name);
    zone.doorTown = readDoor(table, "door_town", name);
    zone.doorFarm = readDoor(table, "door_farm", name);
    zone.doorMines = readDoor(table, "door_mines", name);
    zone.doorExit = readDoor(table, "door_exit", name);
    zone.doorNextLevel = readDoor(table, "door_next_level", name);
    if (const toml::node* houses = table.get("player_houses")) {
        zone.playerHouses = requireInt(houses, name + ".player_houses");
    }
    return zone;
}

MapData MapLoader::load(const std::filesystem::path& path) const {
    const toml::table file = toml::parse_file(path.string());
    MapData map{};
    map.farm = readZone(requireTable(file.get("farm"), "farm"), "farm");
    map.town = readZone(requireTable(file.get("town"), "town"), "town");

    const toml::array& levels = requireArray(file.get("mine_levels"), "mine_levels");
    map.mineLevels.reserve(levels.size());
    for (std::size_t i = 0; i < levels.size(); ++i) {
        const std::string name = "mine_levels[" + std::to_string(i) + "]";
        map.mineLevels.push_back(readZone(requireTable(levels.get(i), name), name));
    }
    return map;
}
