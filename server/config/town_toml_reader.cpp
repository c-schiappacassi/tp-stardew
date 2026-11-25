#include "config/town_toml_reader.h"

#include "server_constants.h"

#include <cstddef>
#include <cstdint>
#include <set>
#include <string>
#include <utility>

#include "config/toml_helpers.h"

TownPosition TownTomlReader::readPosition(const toml::table& table,
                                          const std::string& field) {
    const auto& position = toml_helpers::tableAt(
        table.get(server_constants::POSITION), field + server_constants::FIELD_POSITION);

    return {
        static_cast<uint16_t>(toml_helpers::integer(
            position, server_constants::X, 65535, field + server_constants::FIELD_POSITION_X)),
        static_cast<uint16_t>(toml_helpers::integer(
            position, server_constants::Y, 65535, field + server_constants::FIELD_POSITION_Y))
    };
}

TownConfig TownTomlReader::read(const toml::table& root) const {
    using toml_helpers::integer;
    using toml_helpers::invalid;
    using toml_helpers::string;
    using toml_helpers::tableAt;

    const auto& town = tableAt(root.get(server_constants::TOWN), server_constants::TOWN);
    const auto& map = tableAt(town.get(server_constants::MAP), server_constants::TOWN_MAP);

    TownConfig config;
    config.map.width = static_cast<uint16_t>(
        integer(map, server_constants::WIDTH, 65535, server_constants::TOWN_MAP_WIDTH));
    config.map.height = static_cast<uint16_t>(
        integer(map, server_constants::HEIGHT, 65535, server_constants::TOWN_MAP_HEIGHT));
    config.map.tileSize = static_cast<uint16_t>(
        integer(map, server_constants::TILE_SIZE, 65535, server_constants::TOWN_MAP_TILE_SIZE));

    if (config.map.width == 0 || config.map.height == 0 ||
        config.map.tileSize == 0) {
        invalid(server_constants::TOWN_MAP,
                server_constants::DIMENSIONS_AND_TILE_SIZE_MUST_BE_GREATER_THAN_ZERO);
    }

    const auto* buildingsNode = town.get(server_constants::BUILDINGS);
    if (!buildingsNode || !buildingsNode->is_array()) {
        invalid(server_constants::TOWN_BUILDINGS, server_constants::EXPECTED_AN_ARRAY);
    }

    std::set<std::string> buildingIds;
    const auto& buildings = *buildingsNode->as_array();
    config.buildings.reserve(buildings.size());

    for (std::size_t i = 0; i < buildings.size(); ++i) {
        const auto field = server_constants::TOWN_BUILDINGS_PREFIX + std::to_string(i) + server_constants::CLOSE_BRACKET;
        const auto& building = tableAt(&buildings[i], field);

        TownBuildingConfig entry;
        entry.id = string(building, server_constants::ID, field + server_constants::FIELD_ID);
        entry.position = readPosition(building, field);
        entry.width = static_cast<uint16_t>(
            integer(building, server_constants::WIDTH, 65535, field + server_constants::FIELD_WIDTH));
        entry.height = static_cast<uint16_t>(
            integer(building, server_constants::HEIGHT, 65535, field + server_constants::FIELD_HEIGHT));

        if (entry.id.empty()) {
            invalid(field + server_constants::FIELD_ID, server_constants::THE_IDENTIFIER_CANNOT_BE_EMPTY);
        }
        if (!buildingIds.insert(entry.id).second) {
            invalid(field + server_constants::FIELD_ID, server_constants::DUPLICATE_BUILDING);
        }

        const auto right = static_cast<uint32_t>(entry.position.x) + entry.width;
        const auto bottom = static_cast<uint32_t>(entry.position.y) + entry.height;
        if (entry.width == 0 || entry.height == 0 ||
            right > config.map.width || bottom > config.map.height) {
            invalid(field, server_constants::THE_BUILDING_DOES_NOT_FIT_INSIDE_THE_MAP);
        }

        for (const auto& other : config.buildings) {
            const auto otherRight =
                static_cast<uint32_t>(other.position.x) + other.width;
            const auto otherBottom =
                static_cast<uint32_t>(other.position.y) + other.height;
            const bool overlaps =
                entry.position.x < otherRight && other.position.x < right &&
                entry.position.y < otherBottom && other.position.y < bottom;
            if (overlaps) {
                invalid(field, server_constants::THE_BUILDING_OVERLAPS_ANOTHER_BUILDING);
            }
        }

        config.buildings.push_back(std::move(entry));
    }

    const auto pointInsideBuilding = [&config](const TownPosition& point) {
        for (const auto& building : config.buildings) {
            const auto right = static_cast<uint32_t>(building.position.x) +
                               building.width;
            const auto bottom = static_cast<uint32_t>(building.position.y) +
                                building.height;
            if (point.x >= building.position.x && point.x < right &&
                point.y >= building.position.y && point.y < bottom) {
                return true;
            }
        }
        return false;
    };

    const auto* npcsNode = town.get(server_constants::NPCS);
    if (!npcsNode || !npcsNode->is_array()) {
        invalid(server_constants::TOWN_NPCS, server_constants::EXPECTED_AN_ARRAY);
    }

    std::set<std::string> npcIds;
    const auto& npcs = *npcsNode->as_array();
    config.npcs.reserve(npcs.size());

    for (std::size_t i = 0; i < npcs.size(); ++i) {
        const auto field = server_constants::TOWN_NPCS_PREFIX + std::to_string(i) + server_constants::CLOSE_BRACKET;
        const auto& npc = tableAt(&npcs[i], field);

        TownNpcConfig entry;
        entry.id = string(npc, server_constants::ID, field + server_constants::FIELD_ID);
        entry.position = readPosition(npc, field);

        if (entry.id.empty()) {
            invalid(field + server_constants::FIELD_ID, server_constants::THE_IDENTIFIER_CANNOT_BE_EMPTY);
        }
        if (!npcIds.insert(entry.id).second) {
            invalid(field + server_constants::FIELD_ID, server_constants::DUPLICATE_NPC);
        }
        if (entry.position.x >= config.map.width ||
            entry.position.y >= config.map.height) {
            invalid(field + server_constants::FIELD_POSITION, server_constants::POSITION_IS_OUTSIDE_THE_MAP);
        }
        if (pointInsideBuilding(entry.position)) {
            invalid(field + server_constants::FIELD_POSITION, server_constants::THE_NPC_CANNOT_BE_INSIDE_A_BUILDING);
        }

        config.npcs.push_back(std::move(entry));
    }

    const auto* connectionsNode = town.get(server_constants::CONNECTIONS);
    if (!connectionsNode || !connectionsNode->is_array()) {
        invalid(server_constants::TOWN_CONNECTIONS, server_constants::EXPECTED_AN_ARRAY);
    }

    std::set<std::string> destinations;
    const auto& connections = *connectionsNode->as_array();
    config.connections.reserve(connections.size());

    for (std::size_t i = 0; i < connections.size(); ++i) {
        const auto field = server_constants::TOWN_CONNECTIONS_PREFIX + std::to_string(i) + server_constants::CLOSE_BRACKET;
        const auto& connection = tableAt(&connections[i], field);

        TownConnectionConfig entry;
        entry.destination = string(
            connection, server_constants::DESTINATION, field + server_constants::FIELD_DESTINATION);
        entry.position = readPosition(connection, field);

        if (entry.destination.empty()) {
            invalid(field + server_constants::FIELD_DESTINATION, server_constants::THE_DESTINATION_CANNOT_BE_EMPTY);
        }
        if (!destinations.insert(entry.destination).second) {
            invalid(field + server_constants::FIELD_DESTINATION, server_constants::DUPLICATE_DESTINATION);
        }
        if (entry.position.x >= config.map.width ||
            entry.position.y >= config.map.height) {
            invalid(field + server_constants::FIELD_POSITION, server_constants::POSITION_IS_OUTSIDE_THE_MAP);
        }
        if (pointInsideBuilding(entry.position)) {
            invalid(field + server_constants::FIELD_POSITION,
                    server_constants::THE_CONNECTION_CANNOT_BE_INSIDE_A_BUILDING);
        }

        config.connections.push_back(std::move(entry));
    }

    return config;
}
