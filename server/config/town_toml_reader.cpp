#include "config/town_toml_reader.h"

#include <cstddef>
#include <cstdint>
#include <set>
#include <string>
#include <utility>

#include "config/toml_helpers.h"

TownPosition TownTomlReader::readPosition(const toml::table& table,
                                          const std::string& field) {
    const auto& position = toml_helpers::tableAt(
        table.get("position"), field + ".position");

    return {
        static_cast<uint16_t>(toml_helpers::integer(
            position, "x", 65535, field + ".position.x")),
        static_cast<uint16_t>(toml_helpers::integer(
            position, "y", 65535, field + ".position.y"))
    };
}

TownConfig TownTomlReader::read(const toml::table& root) const {
    using toml_helpers::integer;
    using toml_helpers::invalid;
    using toml_helpers::string;
    using toml_helpers::tableAt;

    const auto& town = tableAt(root.get("town"), "town");
    const auto& map = tableAt(town.get("map"), "town.map");

    TownConfig config;
    config.map.width = static_cast<uint16_t>(
        integer(map, "width", 65535, "town.map.width"));
    config.map.height = static_cast<uint16_t>(
        integer(map, "height", 65535, "town.map.height"));
    config.map.tileSize = static_cast<uint16_t>(
        integer(map, "tile_size", 65535, "town.map.tile_size"));

    if (config.map.width == 0 || config.map.height == 0 ||
        config.map.tileSize == 0) {
        invalid("town.map",
                "las dimensiones y el tile_size deben ser mayores que cero");
    }

    const auto* buildingsNode = town.get("buildings");
    if (!buildingsNode || !buildingsNode->is_array()) {
        invalid("town.buildings", "se esperaba una lista");
    }

    std::set<std::string> buildingIds;
    const auto& buildings = *buildingsNode->as_array();
    config.buildings.reserve(buildings.size());

    for (std::size_t i = 0; i < buildings.size(); ++i) {
        const auto field = "town.buildings[" + std::to_string(i) + "]";
        const auto& building = tableAt(&buildings[i], field);

        TownBuildingConfig entry;
        entry.id = string(building, "id", field + ".id");
        entry.position = readPosition(building, field);
        entry.width = static_cast<uint16_t>(
            integer(building, "width", 65535, field + ".width"));
        entry.height = static_cast<uint16_t>(
            integer(building, "height", 65535, field + ".height"));

        if (entry.id.empty()) {
            invalid(field + ".id", "el identificador no puede estar vacio");
        }
        if (!buildingIds.insert(entry.id).second) {
            invalid(field + ".id", "edificio repetido");
        }

        const auto right = static_cast<uint32_t>(entry.position.x) + entry.width;
        const auto bottom = static_cast<uint32_t>(entry.position.y) + entry.height;
        if (entry.width == 0 || entry.height == 0 ||
            right > config.map.width || bottom > config.map.height) {
            invalid(field, "el edificio no cabe dentro del mapa");
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
                invalid(field, "el edificio se superpone con otro edificio");
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

    const auto* npcsNode = town.get("npcs");
    if (!npcsNode || !npcsNode->is_array()) {
        invalid("town.npcs", "se esperaba una lista");
    }

    std::set<std::string> npcIds;
    const auto& npcs = *npcsNode->as_array();
    config.npcs.reserve(npcs.size());

    for (std::size_t i = 0; i < npcs.size(); ++i) {
        const auto field = "town.npcs[" + std::to_string(i) + "]";
        const auto& npc = tableAt(&npcs[i], field);

        TownNpcConfig entry;
        entry.id = string(npc, "id", field + ".id");
        entry.position = readPosition(npc, field);

        if (entry.id.empty()) {
            invalid(field + ".id", "el identificador no puede estar vacio");
        }
        if (!npcIds.insert(entry.id).second) {
            invalid(field + ".id", "NPC repetido");
        }
        if (entry.position.x >= config.map.width ||
            entry.position.y >= config.map.height) {
            invalid(field + ".position", "posicion fuera del mapa");
        }
        if (pointInsideBuilding(entry.position)) {
            invalid(field + ".position", "el NPC no puede estar dentro de un edificio");
        }

        config.npcs.push_back(std::move(entry));
    }

    const auto* connectionsNode = town.get("connections");
    if (!connectionsNode || !connectionsNode->is_array()) {
        invalid("town.connections", "se esperaba una lista");
    }

    std::set<std::string> destinations;
    const auto& connections = *connectionsNode->as_array();
    config.connections.reserve(connections.size());

    for (std::size_t i = 0; i < connections.size(); ++i) {
        const auto field = "town.connections[" + std::to_string(i) + "]";
        const auto& connection = tableAt(&connections[i], field);

        TownConnectionConfig entry;
        entry.destination = string(
            connection, "destination", field + ".destination");
        entry.position = readPosition(connection, field);

        if (entry.destination.empty()) {
            invalid(field + ".destination", "el destino no puede estar vacio");
        }
        if (!destinations.insert(entry.destination).second) {
            invalid(field + ".destination", "destino repetido");
        }
        if (entry.position.x >= config.map.width ||
            entry.position.y >= config.map.height) {
            invalid(field + ".position", "posicion fuera del mapa");
        }
        if (pointInsideBuilding(entry.position)) {
            invalid(field + ".position",
                    "la conexion no puede estar dentro de un edificio");
        }

        config.connections.push_back(std::move(entry));
    }

    return config;
}
