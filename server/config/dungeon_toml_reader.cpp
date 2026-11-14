#include "config/dungeon_toml_reader.h"

#include <cstddef>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "config/toml_helpers.h"
#include "entities/entity_dimensions.h"
#include "world/zone_map.h"

ResourceType DungeonTomlReader::resourceTypeFromName(
    const std::string& name, const std::string& field) {
    if (name == "CoalRock") return ResourceType::CoalRock;
    if (name == "LapisRock") return ResourceType::LapisRock;
    if (name == "SilverRock") return ResourceType::SilverRock;
    if (name == "GoldRock") return ResourceType::GoldRock;

    toml_helpers::invalid(field, "tipo de mineral desconocido");
}

ItemId DungeonTomlReader::rewardItemFromName(
    const std::string& name, const std::string& field) {
    if (name == "Coal") return ItemId::Coal;
    if (name == "Lapis") return ItemId::Lapis;
    if (name == "Silver") return ItemId::Silver;
    if (name == "Gold") return ItemId::Gold;

    toml_helpers::invalid(field, "objeto de recompensa desconocido");
}

Quality DungeonTomlReader::qualityFromName(
    const std::string& name, const std::string& field) {
    if (name == "None") return Quality::None;

    toml_helpers::invalid(field, "calidad desconocida");
}

Position DungeonTomlReader::readCell(const toml::node* node,
                                     const std::string& field,
                                     int64_t maxX,
                                     int64_t maxY) {
    const auto& position = toml_helpers::tableAt(node, field);
    const auto x = toml_helpers::integer(position, "x", maxX, field + ".x");
    const auto y = toml_helpers::integer(position, "y", maxY, field + ".y");
    return Position(static_cast<int>(x), static_cast<int>(y));
}

Position DungeonTomlReader::readBasePosition(const toml::node* node,
                                             const std::string& field,
                                             const ZoneMap& terrain,
                                             int64_t width,
                                             int64_t height) {
    const auto cell = readCell(node, field, width - 1, height - 1);
    const auto base = terrain.baseAt({cell.getY(), cell.getX()});
    if (!base) {
        toml_helpers::invalid(field, "la celda esta fuera del mapa");
    }
    return *base;
}

DungeonConfig DungeonTomlReader::read(const toml::table& root) const {
    using toml_helpers::integer;
    using toml_helpers::invalid;
    using toml_helpers::string;
    using toml_helpers::tableAt;

    const auto& dungeon = tableAt(root.get("dungeon"), "dungeon");

    DungeonConfig config;
    config.miningRange = static_cast<uint16_t>(integer(
        dungeon, "mining_range", 65535, "dungeon.mining_range"));

    if (config.miningRange == 0) {
        invalid("dungeon.mining_range", "debe ser mayor que cero");
    }

    config.mineralDensityPercent = static_cast<uint8_t>(integer(
        dungeon, "mineral_density_percent", 100,
        "dungeon.mineral_density_percent"));

    config.minerals.clear();
    config.mineralDensityByRoom.clear();
    config.roomLayouts.clear();

    const auto* mapNode = dungeon.get("map");
    const auto* roomsNode = dungeon.get("rooms");
    if (mapNode || roomsNode) {
        const auto& map = tableAt(mapNode, "dungeon.map");
        if (!roomsNode || !roomsNode->is_array()) {
            invalid("dungeon.rooms", "se esperaba una lista de habitaciones");
        }

        const auto width = integer(map, "width", 256, "dungeon.map.width");
        const auto height = integer(map, "height", 256, "dungeon.map.height");
        const auto tileSize = integer(map, "tile_size", 256, "dungeon.map.tile_size");
        if (width == 0 || height == 0 || tileSize == 0) {
            invalid("dungeon.map", "ancho, alto y tile_size deben ser mayores que cero");
        }
        if (tileSize != entity_dimensions::TILE_SIZE) {
            invalid("dungeon.map.tile_size",
                    "debe coincidir con el tamaño de tile del juego");
        }

        bool defaultWalkable = true;
        if (map.get("default_walkable")) {
            defaultWalkable = toml_helpers::boolean(
                map, "default_walkable", "dungeon.map.default_walkable");
        }

        const auto& rooms = *roomsNode->as_array();
        for (std::size_t i = 0; i < rooms.size(); ++i) {
            const auto field = "dungeon.rooms[" + std::to_string(i) + "]";
            const auto& room = tableAt(&rooms[i], field);
            const auto number = static_cast<uint8_t>(integer(
                room, "number", DUNGEON_ROOM_COUNT, field + ".number"));
            if (number == 0) {
                invalid(field + ".number", "la habitacion debe estar entre 1 y 10");
            }

            std::vector<std::vector<bool>> walkable(
                static_cast<std::size_t>(height),
                std::vector<bool>(static_cast<std::size_t>(width), defaultWalkable));

            const auto* walkableNode = room.get("walkable_tiles");
            if (walkableNode) {
                if (!walkableNode->is_array()) {
                    invalid(field + ".walkable_tiles", "se esperaba una lista");
                }
                const auto& walkableCells = *walkableNode->as_array();
                for (std::size_t j = 0; j < walkableCells.size(); ++j) {
                    const auto tileField = field + ".walkable_tiles[" +
                                           std::to_string(j) + "]";
                    const auto cell = readCell(
                        &walkableCells[j], tileField, width - 1, height - 1);
                    walkable[cell.getY()][cell.getX()] = true;
                }
            }

            const auto* blockedNode = room.get("blocked_tiles");
            if (blockedNode) {
                if (!blockedNode->is_array()) {
                    invalid(field + ".blocked_tiles", "se esperaba una lista");
                }
                const auto& blocked = *blockedNode->as_array();
                for (std::size_t j = 0; j < blocked.size(); ++j) {
                    const auto tileField = field + ".blocked_tiles[" + std::to_string(j) + "]";
                    const auto cell = readCell(
                        &blocked[j], tileField, width - 1, height - 1);
                    walkable[cell.getY()][cell.getX()] = false;
                }
            }

            DungeonRoomLayout layout;
            layout.terrain = ZoneMap(walkable, static_cast<int>(tileSize));
            layout.passages.entry = readBasePosition(
                room.get("entry"), field + ".entry", layout.terrain, width, height);
            layout.passages.exit = readBasePosition(
                room.get("exit"), field + ".exit", layout.terrain, width, height);
            if (room.get("next_room")) {
                layout.passages.nextRoom = readBasePosition(
                    room.get("next_room"), field + ".next_room",
                    layout.terrain, width, height);
            }

            const auto* reservedNode = room.get("reserved_positions");
            if (reservedNode) {
                if (!reservedNode->is_array()) {
                    invalid(field + ".reserved_positions", "se esperaba una lista");
                }
                const auto& reserved = *reservedNode->as_array();
                for (std::size_t j = 0; j < reserved.size(); ++j) {
                    const auto positionField = field + ".reserved_positions[" +
                                               std::to_string(j) + "]";
                    layout.reservedPositions.push_back(readBasePosition(
                        &reserved[j], positionField, layout.terrain, width, height));
                }
            }

            if (!config.roomLayouts.emplace(number, std::move(layout)).second) {
                invalid(field + ".number", "numero de habitacion repetido");
            }
        }

        if (config.roomLayouts.size() != DUNGEON_ROOM_COUNT) {
            invalid("dungeon.rooms", "deben configurarse exactamente 10 habitaciones");
        }
    }

    const auto* densityNode = dungeon.get("mineral_density_by_room");
    if (!densityNode || !densityNode->is_array()) {
        invalid("dungeon.mineral_density_by_room", "se esperaba una lista");
    }

    const auto& densities = *densityNode->as_array();
    for (std::size_t i = 0; i < densities.size(); ++i) {
        const auto field = "dungeon.mineral_density_by_room[" +
                           std::to_string(i) + "]";
        const auto& density = tableAt(&densities[i], field);

        const auto room = static_cast<uint8_t>(integer(
            density, "room", DUNGEON_ROOM_COUNT, field + ".room"));
        const auto percentage = static_cast<uint8_t>(integer(
            density, "density", 100, field + ".density"));

        if (room == 0) {
            invalid(field + ".room", "la habitacion debe ser mayor que cero");
        }

        if (!config.mineralDensityByRoom.emplace(room, percentage).second) {
            invalid(field, "habitacion repetida");
        }
    }

    const auto* mineralsNode = dungeon.get("minerals");
    if (!mineralsNode || !mineralsNode->is_array()) {
        invalid("dungeon.minerals", "se esperaba una lista");
    }

    const auto& minerals = *mineralsNode->as_array();
    if (minerals.empty()) {
        invalid("dungeon.minerals", "debe haber al menos un mineral");
    }

    config.minerals.reserve(minerals.size());
    uint64_t totalSpawnWeight = 0;

    for (std::size_t i = 0; i < minerals.size(); ++i) {
        const auto field = "dungeon.minerals[" + std::to_string(i) + "]";
        const auto& mineral = tableAt(&minerals[i], field);

        const auto typeName = string(mineral, "type", field + ".type");
        const auto type = resourceTypeFromName(typeName, field + ".type");

        const auto hits = integer(
            mineral, "hits_to_break", 65535, field + ".hits_to_break");
        if (hits == 0) {
            invalid(field + ".hits_to_break", "debe ser mayor que cero");
        }

        const auto weight = integer(
            mineral, "spawn_weight", 65535, field + ".spawn_weight");
        totalSpawnWeight += static_cast<uint64_t>(weight);

        const auto firstRoom = integer(
            mineral, "first_room", DUNGEON_ROOM_COUNT, field + ".first_room");
        if (firstRoom == 0) {
            invalid(field + ".first_room",
                    "la habitacion debe ser mayor que cero");
        }

        const auto itemName = string(
            mineral, "reward_item", field + ".reward_item");
        const auto item = rewardItemFromName(itemName, field + ".reward_item");

        const auto quantity = integer(
            mineral, "reward_quantity", 65535, field + ".reward_quantity");
        if (quantity == 0) {
            invalid(field + ".reward_quantity", "debe ser mayor que cero");
        }

        const auto qualityName = string(
            mineral, "reward_quality", field + ".reward_quality");
        const auto quality = qualityFromName(
            qualityName, field + ".reward_quality");

        const auto chance = integer(
            mineral, "reward_chance", 100, field + ".reward_chance");

        config.minerals.push_back({
            type,
            static_cast<uint16_t>(hits),
            static_cast<uint16_t>(weight),
            static_cast<uint8_t>(firstRoom),
            {item, static_cast<uint16_t>(quantity), quality},
            static_cast<uint8_t>(chance)
        });
    }

    if (totalSpawnWeight == 0) {
        invalid("dungeon.minerals", "al menos un mineral debe tener spawn_weight mayor que cero");
    }

    return config;
}
