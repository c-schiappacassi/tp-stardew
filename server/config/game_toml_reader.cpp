#include "config/game_toml_reader.h"

#include <algorithm>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <toml++/toml.hpp>
#include <vector>
#include <utility>

#include "entities/entity_id_generator.h"
#include "entities/entity_dimensions.h"
#include "farm/farm_manager.h"


namespace {

[[noreturn]] void invalid(const std::string& field,
                          const std::string& reason) {
    throw std::invalid_argument(field + ": " + reason);
}

const toml::table& tableAt(const toml::node* node,
                           std::string_view field) {
    if (!node || !node->is_table()) {
        invalid(std::string(field), "se esperaba una tabla");
    }

    return *node->as_table();
}

int64_t integer(const toml::table& table, std::string_view key,
                int64_t maximum, const std::string& field) {
    const auto* node = table.get(key);

    if (!node || !node->is_integer()) {
        invalid(field, "se esperaba un entero");
    }

    const auto value = *node->value<int64_t>();

    if (value < 0 || value > maximum) {
        invalid(field, "entero fuera de rango");
    }

    return value;
}

bool boolean(const toml::table& table, std::string_view key,
             const std::string& field) {
    const auto* node = table.get(key);

    if (!node || !node->is_boolean()) {
        invalid(field, "se esperaba un booleano");
    }

    return *node->value<bool>();
}

std::string string(const toml::table& table, std::string_view key,
                   const std::string& field) {
    const auto* node = table.get(key);

    if (!node || !node->is_string()) {
        invalid(field, "se esperaba un texto");
    }

    return *node->value<std::string>();
}

constexpr int TILE_SIZE = entity_dimensions::TILE_SIZE;
constexpr int MAX_FARM_DIMENSION =
    std::numeric_limits<uint16_t>::max() / TILE_SIZE;
constexpr std::size_t MAX_FARM_TILES = 1024 * 1024;

bool optionalBool(const toml::table& table,
                  std::string_view key,
                  const std::string& field) {
    const auto* node = table.get(key);

    if (!node) return false;

    if (!node->is_boolean()) {
        invalid(field, "se esperaba un booleano");
    }

    return *node->value<bool>();
}

template <class Callback>
void farmEntries(const toml::table& table,
                 std::string_view key,
                 Callback callback,
                 bool required = false,
                 std::size_t limit =
                     std::numeric_limits<uint16_t>::max()) {
    const auto field = "farm." + std::string(key);
    const auto* node = table.get(key);

    if (!node && !required) return;

    if (!node || !node->is_array()) {
        invalid(field, "se esperaba una lista");
    }

    const auto& array = *node->as_array();

    if (array.size() > limit) {
        invalid(field, "demasiados elementos");
    }

    for (std::size_t i = 0; i < array.size(); ++i) {
        const auto item = field + "[" + std::to_string(i) + "]";
        callback(tableAt(&array[i], item), item);
    }
}

Position farmTileAt(const toml::table& table,
                    int width,
                    int height,
                    const std::string& field) {
    return Position{
        static_cast<int>(integer(table, "x", width - 1, field + ".x")),
        static_cast<int>(integer(table, "y", height - 1, field + ".y"))
    };
}

Position farmBaseAt(const toml::node* node,
                    const ZoneMap& terrain,
                    int width,
                    int height,
                    const std::string& field) {
    const auto tile = farmTileAt(
        tableAt(node, field), width, height, field);

    const auto base = *terrain.baseAt({
        tile.getY(), tile.getX()
    });

    if (!terrain.isWalkableBase(base)) {
        invalid(field, "el tile es void o unwalkable");
    }

    return base;
}

template <class Type, std::size_t N>
Type farmNamedType(
    const toml::table& table,
    const std::pair<std::string_view, Type> (&types)[N],
    const std::string& field) {

    const auto name = string(table, "type", field + ".type");

    for (const auto& [label, type] : types) {
        if (label == name) return type;
    }

    invalid(field + ".type", "tipo desconocido: " + name);
}

void reserveFarmPosition(FarmLayout& layout,
                         const Position& base) {
    const auto alreadyReserved = std::any_of(
        layout.reservedPositions.begin(),
        layout.reservedPositions.end(),
        [&](const auto& other) {
            return layout.terrain.sameBaseCell(base, other);
        });

    if (!alreadyReserved) {
        layout.reservedPositions.push_back(base);
    }
}

FarmLayout readFarmLayout(const toml::table& root,
                          const FarmConfig& config) {
    const auto& farm = tableAt(root.get("farm"), "farm");

    const int width = static_cast<int>(
        integer(farm, "width", MAX_FARM_DIMENSION, "farm.width"));

    const int height = static_cast<int>(
        integer(farm, "height", MAX_FARM_DIMENSION, "farm.height"));

    if (width == 0 || height == 0 ||
        static_cast<std::size_t>(width) * height > MAX_FARM_TILES) {
        invalid("farm", "dimensiones vacias o demasiado grandes");
    }

    FarmLayout layout;

    std::vector<std::vector<bool>> walkable(
        height, std::vector<bool>(width, false));

    std::vector<std::vector<bool>> defined(
        height, std::vector<bool>(width, false));

    farmEntries(farm, "background",
        [&](const auto& entry, const auto& field) {
            const auto tile = farmTileAt(entry, width, height, field);
            const int x = tile.getX();
            const int y = tile.getY();

            if (defined[y][x]) {
                invalid(field, "tile repetido");
            }

            defined[y][x] = true;

            const auto state = string(
                entry, "state", field + ".state");

            if (state != "walkable" && state != "unwalkable") {
                invalid(field + ".state",
                        "se esperaba walkable o unwalkable");
            }

            walkable[y][x] = state == "walkable";

            const auto texture = integer(
                entry, "texture_id",
                std::numeric_limits<uint16_t>::max(),
                field + ".texture_id");

            layout.background.push_back({
                tile, static_cast<uint16_t>(texture)
            });
        }, true, MAX_FARM_TILES);

    layout.terrain = ZoneMap{walkable, TILE_SIZE};

    layout.spawn = farmBaseAt(
        farm.get("spawn"), layout.terrain,
        width, height, "farm.spawn");

    reserveFarmPosition(layout, layout.spawn);

    if (farm.contains("player_houses")) {
        const int count = static_cast<int>(
            integer(farm, "player_houses", 4, "farm.player_houses"));

        if (count == 0) {
            invalid("farm.player_houses",
                    "debe haber entre 1 y 4 casas");
        }

        layout.playerHouses = static_cast<uint8_t>(count);
    }

    if (farm.contains("door_mines")) {
        layout.mineDoor = farmBaseAt(
            farm.get("door_mines"), layout.terrain,
            width, height, "farm.door_mines");

        layout.mineReturn = farm.contains("mine_return")
            ? farmBaseAt(farm.get("mine_return"), layout.terrain,
                         width, height, "farm.mine_return")
            : layout.spawn;

        reserveFarmPosition(layout, *layout.mineDoor);
        reserveFarmPosition(layout, *layout.mineReturn);

    } else if (farm.contains("mine_return")) {
        invalid("farm.mine_return", "requiere door_mines");
    }

    if (farm.contains("door_town")) {
        reserveFarmPosition(
            layout,
            farmBaseAt(farm.get("door_town"), layout.terrain,
                       width, height, "farm.door_town"));
    }

    farmEntries(farm, "reserved_positions",
        [&](const auto& entry, const auto& field) {
            reserveFarmPosition(
                layout,
                farmBaseAt(&entry, layout.terrain,
                           width, height, field));
        });

    for (const auto& base : layout.reservedPositions) {
        if (!layout.terrain.hasBasePath(layout.spawn, base)) {
            invalid("farm",
                    "spawn y accesos reservados no estan conectados");
        }
    }

    farmEntries(farm, "tilled_soil",
        [&](const auto& entry, const auto& field) {
            layout.tilledSoil.push_back(
                farmBaseAt(&entry, layout.terrain,
                           width, height, field));
        });

    constexpr std::pair<std::string_view, CropType> cropTypes[]{
        {"strawberry", CropType::Strawberry},
        {"scallion", CropType::Scallion},
        {"potato", CropType::Potato},
        {"pumpkin", CropType::Pumpkin}
    };

    farmEntries(farm, "crops",
        [&](const auto& entry, const auto& field) {
            if (entry.contains("growth_stage")) {
                invalid(field + ".growth_stage",
                        "la carga inicial crea cultivos nuevos; "
                        "no restaura crecimiento");
            }

            layout.crops.push_back({
                farmBaseAt(entry.get("position"), layout.terrain,
                           width, height, field + ".position"),
                farmNamedType(entry, cropTypes, field),
                optionalBool(entry, "watered",
                             field + ".watered")
            });
        });

    constexpr std::pair<std::string_view, AnimalType> animalTypes[]{
        {"brown_cow", AnimalType::BrownCow},
        {"white_cow", AnimalType::WhiteCow},
        {"brown_chicken", AnimalType::BrownChicken},
        {"white_chicken", AnimalType::WhiteChicken},
        {"blue_chicken", AnimalType::BlueChicken},
        {"void_chicken", AnimalType::VoidChicken}
    };

    farmEntries(farm, "animals",
        [&](const auto& entry, const auto& field) {
            layout.animals.push_back({
                farmBaseAt(entry.get("position"), layout.terrain,
                           width, height, field + ".position"),
                farmNamedType(entry, animalTypes, field)
            });
        });

    farmEntries(farm, "furniture",
        [&](const auto& entry, const auto& field) {
            const auto item = static_cast<ItemId>(
                integer(entry, "item_id",
                        std::numeric_limits<uint16_t>::max(),
                        field + ".item_id"));

            if (!Decoration::isDecoration(item)) {
                invalid(field + ".item_id", "mueble no soportado");
            }

            layout.decorations.push_back({
                farmBaseAt(entry.get("position"), layout.terrain,
                           width, height, field + ".position"),
                item
            });
        });

    farmEntries(farm, "chests",
        [](const auto&, const auto& field) {
            invalid(field,
                    "la carga de cofres con inventario "
                    "todavia no esta implementada");
        });

    FarmManager prepared{config, 0};
    EntityIdGenerator ids;

    if (!prepared.initialize(layout, ids)) {
        invalid("farm",
                "distribucion invalida: revisar entidades "
                "superpuestas, limites y tiles reservados");
    }

    return layout;
}

TownPosition readTownPosition(const toml::table& table,
                              const std::string& field) {
    const auto& position = tableAt(
        table.get("position"), field + ".position");

    return {
        static_cast<uint16_t>(
            integer(position, "x", 65535, field + ".position.x")),
        static_cast<uint16_t>(
            integer(position, "y", 65535, field + ".position.y"))
    };
}

ResourceType resourceTypeFromName(const std::string& name,
                                  const std::string& field) {
    if (name == "CoalRock") return ResourceType::CoalRock;
    if (name == "LapisRock") return ResourceType::LapisRock;
    if (name == "SilverRock") return ResourceType::SilverRock;
    if (name == "GoldRock") return ResourceType::GoldRock;

    invalid(field, "tipo de mineral desconocido");
}

ItemId rewardItemFromName(const std::string& name,
                          const std::string& field) {
    if (name == "Coal") return ItemId::Coal;
    if (name == "Lapis") return ItemId::Lapis;
    if (name == "Silver") return ItemId::Silver;
    if (name == "Gold") return ItemId::Gold;

    invalid(field, "objeto de recompensa desconocido");
}

Quality qualityFromName(const std::string& name,
                        const std::string& field) {
    if (name == "None") return Quality::None;

    invalid(field, "calidad desconocida");
}

void readDungeon(const toml::table& root, GameConfig& config) {
    const auto& dungeon = tableAt(root.get("dungeon"), "dungeon");

    config.dungeon.miningRange = static_cast<uint16_t>(
        integer(dungeon, "mining_range", 65535,
                "dungeon.mining_range"));

    if (config.dungeon.miningRange == 0) {
        invalid("dungeon.mining_range", "debe ser mayor que cero");
    }

    config.dungeon.mineralDensityPercent = static_cast<uint8_t>(
        integer(dungeon, "mineral_density_percent", 100,
                "dungeon.mineral_density_percent"));

    config.dungeon.minerals.clear();
    config.dungeon.mineralDensityByRoom.clear();

    const auto* densityNode = dungeon.get("mineral_density_by_room");

    if (!densityNode || !densityNode->is_array()) {
        invalid("dungeon.mineral_density_by_room",
                "se esperaba una lista");
    }

    const auto& densities = *densityNode->as_array();

    for (std::size_t i = 0; i < densities.size(); ++i) {
        const auto field = "dungeon.mineral_density_by_room[" +
                           std::to_string(i) + "]";
        const auto& density = tableAt(&densities[i], field);

        const auto room = static_cast<uint8_t>(
            integer(density, "room", DUNGEON_ROOM_COUNT,
                    field + ".room"));

        const auto percentage = static_cast<uint8_t>(
            integer(density, "density", 100,
                    field + ".density"));

        if (room == 0) {
            invalid(field + ".room", "la habitacion debe ser mayor que cero");
        }

        if (!config.dungeon.mineralDensityByRoom.emplace(
                room, percentage).second) {
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

    for (std::size_t i = 0; i < minerals.size(); ++i) {
        const auto field = "dungeon.minerals[" + std::to_string(i) + "]";
        const auto& mineral = tableAt(&minerals[i], field);

        const auto typeName = string(mineral, "type", field + ".type");
        const auto type = resourceTypeFromName(
            typeName, field + ".type");

        const auto hits = integer(
            mineral, "hits_to_break", 65535,
            field + ".hits_to_break");

        if (hits == 0) {
            invalid(field + ".hits_to_break",
                    "debe ser mayor que cero");
        }

        const auto weight = integer(
            mineral, "spawn_weight", 65535,
            field + ".spawn_weight");

        const auto firstRoom = integer(
            mineral, "first_room", DUNGEON_ROOM_COUNT,
            field + ".first_room");

        if (firstRoom == 0) {
            invalid(field + ".first_room",
                    "la habitacion debe ser mayor que cero");
        }

        const auto itemName = string(
            mineral, "reward_item", field + ".reward_item");

        const auto item = rewardItemFromName(
            itemName, field + ".reward_item");

        const auto quantity = integer(
            mineral, "reward_quantity", 65535,
            field + ".reward_quantity");

        if (quantity == 0) {
            invalid(field + ".reward_quantity",
                    "debe ser mayor que cero");
        }

        const auto qualityName = string(
            mineral, "reward_quality", field + ".reward_quality");

        const auto quality = qualityFromName(
            qualityName, field + ".reward_quality");

        const auto chance = integer(
            mineral, "reward_chance", 100,
            field + ".reward_chance");

        config.dungeon.minerals.push_back({
            type,
            static_cast<uint16_t>(hits),
            static_cast<uint16_t>(weight),
            static_cast<uint8_t>(firstRoom),
            {item, static_cast<uint16_t>(quantity), quality},
            static_cast<uint8_t>(chance)
        });
    }
}

void readTown(const toml::table& root, GameConfig& config) {
    const auto& town = tableAt(root.get("town"), "town");
    const auto& map = tableAt(town.get("map"), "town.map");

    config.town.map.width = static_cast<uint16_t>(
        integer(map, "width", 65535, "town.map.width"));

    config.town.map.height = static_cast<uint16_t>(
        integer(map, "height", 65535, "town.map.height"));

    config.town.map.tileSize = static_cast<uint16_t>(
        integer(map, "tile_size", 65535, "town.map.tile_size"));

    if (config.town.map.width == 0 ||
        config.town.map.height == 0 ||
        config.town.map.tileSize == 0) {
        invalid("town.map", "las dimensiones y el tile_size deben ser mayores que cero");
    }

    const auto* buildingsNode = town.get("buildings");

    if (!buildingsNode || !buildingsNode->is_array()) {
        invalid("town.buildings", "se esperaba una lista");
    }

    std::set<std::string> buildingIds;
    const auto& buildings = *buildingsNode->as_array();

    for (std::size_t i = 0; i < buildings.size(); ++i) {
        const auto field = "town.buildings[" + std::to_string(i) + "]";
        const auto& building = tableAt(&buildings[i], field);

        TownBuildingConfig entry;
        entry.id = string(building, "id", field + ".id");
        entry.position = readTownPosition(building, field);
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

        if (entry.width == 0 || entry.height == 0 ||
            static_cast<uint32_t>(entry.position.x) + entry.width >
                config.town.map.width ||
            static_cast<uint32_t>(entry.position.y) + entry.height >
                config.town.map.height) {
            invalid(field, "el edificio no cabe dentro del mapa");
        }

        config.town.buildings.push_back(entry);
    }

    const auto* npcsNode = town.get("npcs");

    if (!npcsNode || !npcsNode->is_array()) {
        invalid("town.npcs", "se esperaba una lista");
    }

    std::set<std::string> npcIds;
    const auto& npcs = *npcsNode->as_array();

    for (std::size_t i = 0; i < npcs.size(); ++i) {
        const auto field = "town.npcs[" + std::to_string(i) + "]";
        const auto& npc = tableAt(&npcs[i], field);

        TownNpcConfig entry;
        entry.id = string(npc, "id", field + ".id");
        entry.position = readTownPosition(npc, field);

        if (entry.id.empty()) {
            invalid(field + ".id", "el identificador no puede estar vacio");
        }

        if (!npcIds.insert(entry.id).second) {
            invalid(field + ".id", "NPC repetido");
        }

        if (entry.position.x >= config.town.map.width ||
            entry.position.y >= config.town.map.height) {
            invalid(field + ".position", "posicion fuera del mapa");
        }

        config.town.npcs.push_back(entry);
    }

    const auto* connectionsNode = town.get("connections");

    if (!connectionsNode || !connectionsNode->is_array()) {
        invalid("town.connections", "se esperaba una lista");
    }

    std::set<std::string> destinations;
    const auto& connections = *connectionsNode->as_array();

    for (std::size_t i = 0; i < connections.size(); ++i) {
        const auto field = "town.connections[" + std::to_string(i) + "]";
        const auto& connection = tableAt(&connections[i], field);

        TownConnectionConfig entry;
        entry.destination = string(
            connection, "destination", field + ".destination");
        entry.position = readTownPosition(connection, field);

        if (entry.destination.empty()) {
            invalid(field + ".destination",
                    "el destino no puede estar vacio");
        }

        if (!destinations.insert(entry.destination).second) {
            invalid(field + ".destination", "destino repetido");
        }

        if (entry.position.x >= config.town.map.width ||
            entry.position.y >= config.town.map.height) {
            invalid(field + ".position", "posicion fuera del mapa");
        }

        config.town.connections.push_back(entry);
    }
}

ItemId itemFromId(int id, const std::string& field) {
    switch (id) {
        case 50: return ItemId::Chest;
        case 51: return ItemId::Table;
        case 52: return ItemId::Chair;
        case 53: return ItemId::Lamp;
        case 54: return ItemId::Rug;
        case 55: return ItemId::Plant;
        case 56: return ItemId::Painting;
        default:
            invalid(field, "ID de producto desconocido");
    }
}

void readDeath(const toml::table& root, GameConfig& config) {
    const auto& death = tableAt(root.get("death"), "death");

    config.death.goldDrop = static_cast<uint32_t>(
        integer(death, "gold_drop",
                std::numeric_limits<uint32_t>::max(), "death.gold_drop"));
}

void readRobin(const toml::table& root, GameConfig& config) {
    const auto& shops = tableAt(root.get("shops"), "shops");
    const auto& robin = tableAt(shops.get("robin"), "shops.robin");

    config.robin.dailyRotationCount = static_cast<uint8_t>(
        integer(robin, "daily_rotation_count", 255,
                "shops.robin.daily_rotation_count"));

    const auto* node = robin.get("products");

    if (!node || !node->is_array()) {
        invalid("shops.robin.products", "se esperaba una lista");
    }

    const auto& products = *node->as_array();
    std::set<ItemId> registered;

    for (std::size_t i = 0; i < products.size(); ++i) {
        const auto field = "shops.robin.products[" + std::to_string(i) + "]";
        const auto& product = tableAt(&products[i], field);

        const int id = integer(product, "item_id", 65535,
                               field + ".item_id");

        const ItemId item = itemFromId(id, field + ".item_id");

        const int price = integer(product, "price",
                                  std::numeric_limits<uint32_t>::max(),
                                  field + ".price");

        const bool rotates = boolean(product, "rotates",
                                      field + ".rotates");

        if (!registered.insert(item).second) {
            invalid(field, "producto repetido");
        }

        if (item == ItemId::Chest && rotates) {
            invalid(field, "el cofre no puede rotar");
        }

        if (item != ItemId::Chest && !rotates) {
            invalid(field, "las decoraciones deben rotar");
        }

        config.robin.products.push_back({
            item, static_cast<uint32_t>(price), rotates
        });
    }
}

}

WorldData GameTomlReader::read(const std::string& path) const {
    try {
        const auto root = toml::parse_file(path);

        WorldData world;

        readDeath(root, world.config);
        readRobin(root, world.config);
        readDungeon(root, world.config);

        if (root.contains("town")) {
            readTown(root, world.config);
        }

        if (root.contains("farm")) {
            world.farm = readFarmLayout(root, world.config.farm);
        }

        return world;

    } catch (const toml::parse_error& error) {
        std::ostringstream message;
        message << path << ": " << error;
        throw std::invalid_argument(message.str());

    } catch (const std::invalid_argument& error) {
        throw std::invalid_argument(path + ": " + error.what());
    }
}
