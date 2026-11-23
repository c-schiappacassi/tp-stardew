#pragma once

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <toml++/toml.hpp>

#include "common/map/map_data.h"

class MapLoader {
public:
    MapData load(const std::filesystem::path& path) const;

private:
    static std::runtime_error invalidField(const std::string& field);
    static const toml::table& requireTable(const toml::node* node, const std::string& field);
    static const toml::array& requireArray(const toml::node* node, const std::string& field);
    static int requireInt(const toml::node* node, const std::string& field);
    static const toml::node* at(const toml::array& array, std::size_t index);
    static void requireSize(const toml::array& array, std::size_t expected, const std::string& field);

    static TilePosition readPosition(const toml::array& array, const std::string& field);
    static std::optional<TilePosition> readDoor(const toml::table& table, const char* key,
                                                const std::string& zone);
    static std::vector<FloorTile> readBackground(const toml::table& table, const std::string& zone);
    static std::vector<MapElement> readElements(const toml::table& table, const char* key,
                                                const std::string& zone);
    static std::vector<EditorBuildingData> readBuildings(const toml::table& table, const std::string& zone);
    static std::vector<TilePosition> readLockedTiles(const toml::table& table, const std::string& zone);
    static EditorZoneData readZone(const toml::table& table, const std::string& name);
};
