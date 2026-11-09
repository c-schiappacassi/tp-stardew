#include "farm_toml_reader.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <toml++/toml.hpp>

#include "entities/entity_id_generator.h"
#include "farm/farm_manager.h"

namespace {
constexpr int TILE_SIZE = entity_dimensions::TILE_SIZE;
constexpr int MAX_DIMENSION = std::numeric_limits<uint16_t>::max() / TILE_SIZE;
constexpr std::size_t MAX_TILES = 1024 * 1024;

[[noreturn]] void invalid(const std::string& field, const std::string& reason) {
    throw std::invalid_argument(field + ": " + reason);
}

const toml::table& tableAt(const toml::node* node, const std::string& field) {
    if (!node || !node->is_table()) invalid(field, "se esperaba una tabla");
    return *node->as_table();
}

int integer(const toml::table& table, std::string_view key, int maximum,const std::string& field) {
    const auto* node = table.get(key);
    if (!node || !node->is_integer()) invalid(field, "se esperaba un entero");
    const auto value = *node->value<int64_t>();
    if (value < 0 || value > maximum) invalid(field, "entero fuera de rango");
    return static_cast<int>(value);
}

std::string string(const toml::table& table, std::string_view key,const std::string& field) {
    const auto* node = table.get(key);
    if (!node || !node->is_string()) invalid(field, "se esperaba un texto");
    return *node->value<std::string>();
}

bool optionalBool(const toml::table& table, std::string_view key, const std::string& field) {
    const auto* node = table.get(key);
    if (!node) return false;
    if (!node->is_boolean()) invalid(field, "se esperaba un booleano");
    return *node->value<bool>();
}

template <class Callback>
void entries(const toml::table& table, std::string_view key, Callback callback,bool required = false, std::size_t limit = std::numeric_limits<uint16_t>::max()) {
    const auto field = "farm." + std::string(key);
    const auto* node = table.get(key);
    if (!node && !required) return;
    if (!node || !node->is_array()) invalid(field, "se esperaba una lista");
    const auto& array = *node->as_array();
    if (array.size() > limit) invalid(field, "demasiados elementos");
    for (std::size_t i = 0; i < array.size(); ++i) {
        const auto item = field + "[" + std::to_string(i) + "]";
        callback(tableAt(&array[i], item), item);
    }
}

Position tileAt(const toml::table& table, int width, int height, const std::string& field) {
    return Position{integer(table, "x", width - 1, field + ".x"),integer(table, "y", height - 1, field + ".y")};
}

Position baseAt(const toml::node* node, const ZoneMap& terrain, int width, int height,const std::string& field) {
    const auto tile = tileAt(tableAt(node, field), width, height, field);
    const auto base = *terrain.baseAt({tile.getY(), tile.getX()});
    if (!terrain.isWalkableBase(base)) invalid(field, "el tile es void o unwalkable");
    return base;
}

template <class Type, std::size_t N>
Type namedType(const toml::table& table, const std::pair<std::string_view, Type> (&types)[N],const std::string& field) {
    const auto name = string(table, "type", field + ".type");
    for (const auto& [label, type] : types) {
        if (label == name) return type;
    }
    invalid(field + ".type", "tipo desconocido: " + name);
}

void reserve(FarmLayout& layout, const Position& base) {
    const auto alreadyReserved = std::any_of(layout.reservedPositions.begin(),
        layout.reservedPositions.end(), [&](const auto& other) {
            return layout.terrain.sameBaseCell(base, other);
        });
    if (!alreadyReserved) layout.reservedPositions.push_back(base);
}

FarmLayout readFarm(const toml::table& root, const FarmConfig& config) {
    const std::string farmField = "farm";
    const auto& farm = tableAt(root.get("farm"), farmField);
    const int width = integer(farm, "width", MAX_DIMENSION, "farm.width");
    const int height = integer(farm, "height", MAX_DIMENSION, "farm.height");
    if (width == 0 || height == 0 || static_cast<std::size_t>(width) * height > MAX_TILES) {
        invalid("farm", "dimensiones vacias o demasiado grandes");
    }
    FarmLayout layout;
    std::vector<std::vector<bool>> walkable(height, std::vector<bool>(width, false));
    std::vector<std::vector<bool>> defined(height, std::vector<bool>(width, false));
    entries(farm, "background", [&](const auto& entry, const auto& field) {
        const auto tile = tileAt(entry, width, height, field);
        const int x = tile.getX();
        const int y = tile.getY();
        if (defined[y][x]) invalid(field, "tile repetido");
        defined[y][x] = true;
        const auto state = string(entry, "state", field + ".state");
        if (state != "walkable" && state != "unwalkable") {
            invalid(field + ".state", "se esperaba walkable o unwalkable");
        }
        walkable[y][x] = state == "walkable";
        const auto texture = integer(entry, "texture_id", std::numeric_limits<uint16_t>::max(),
                                     field + ".texture_id");
        layout.background.push_back({tile, static_cast<uint16_t>(texture)});
    }, true, MAX_TILES);
    layout.terrain = ZoneMap{walkable, TILE_SIZE};
    layout.spawn = baseAt(farm.get("spawn"), layout.terrain, width, height, "farm.spawn");
    reserve(layout, layout.spawn);

    if (farm.contains("player_houses")) {
        const int count = integer(farm, "player_houses", 4, "farm.player_houses");
        if (count == 0) invalid("farm.player_houses", "debe haber entre 1 y 4 casas");
        layout.playerHouses = static_cast<uint8_t>(count);
    }
    if (farm.contains("door_mines")) {
        layout.mineDoor = baseAt(farm.get("door_mines"), layout.terrain, width, height,
                                 "farm.door_mines");
        layout.mineReturn = farm.contains("mine_return") ?
            baseAt(farm.get("mine_return"), layout.terrain, width, height, "farm.mine_return") :
            layout.spawn;
        reserve(layout, *layout.mineDoor);
        reserve(layout, *layout.mineReturn);
    } else if (farm.contains("mine_return")) {
        invalid("farm.mine_return", "requiere door_mines");
    }
    if (farm.contains("door_town")) {
        reserve(layout, baseAt(farm.get("door_town"), layout.terrain, width, height,
                               "farm.door_town"));
    }
    entries(farm, "reserved_positions", [&](const auto& entry, const auto& field) {
        reserve(layout, baseAt(&entry, layout.terrain, width, height, field));
    });
    for (const auto& base : layout.reservedPositions) {
        if (!layout.terrain.hasBasePath(layout.spawn, base)) {
            invalid("farm", "spawn y accesos reservados no estan conectados");
        }
    }
    entries(farm, "tilled_soil", [&](const auto& entry, const auto& field) {
        layout.tilledSoil.push_back(baseAt(&entry, layout.terrain, width, height, field));
    });

    constexpr std::pair<std::string_view, CropType> cropTypes[]{
        {"strawberry", CropType::Strawberry}, {"scallion", CropType::Scallion},
        {"potato", CropType::Potato}, {"pumpkin", CropType::Pumpkin}};
    entries(farm, "crops", [&](const auto& entry, const auto& field) {
        if (entry.contains("growth_stage")) {
            invalid(field + ".growth_stage", "la carga inicial crea cultivos nuevos; no restaura crecimiento");
        }
        layout.crops.push_back({
            baseAt(entry.get("position"), layout.terrain, width, height, field + ".position"),
            namedType(entry, cropTypes, field), optionalBool(entry, "watered", field + ".watered")});
    });
    constexpr std::pair<std::string_view, AnimalType> animalTypes[]{
        {"brown_cow", AnimalType::BrownCow}, {"white_cow", AnimalType::WhiteCow},
        {"brown_chicken", AnimalType::BrownChicken}, {"white_chicken", AnimalType::WhiteChicken},
        {"blue_chicken", AnimalType::BlueChicken}, {"void_chicken", AnimalType::VoidChicken}};
    entries(farm, "animals", [&](const auto& entry, const auto& field) {
        layout.animals.push_back({
            baseAt(entry.get("position"), layout.terrain, width, height, field + ".position"),
            namedType(entry, animalTypes, field)});
    });
    entries(farm, "furniture", [&](const auto& entry, const auto& field) {
        const auto item = static_cast<ItemId>(integer(entry, "item_id",
            std::numeric_limits<uint16_t>::max(), field + ".item_id"));
        if (!Decoration::isDecoration(item)) invalid(field + ".item_id", "mueble no soportado");
        layout.decorations.push_back({
            baseAt(entry.get("position"), layout.terrain, width, height, field + ".position"), item});
    });
    entries(farm, "chests", [](const auto&, const auto& field) {
        invalid(field, "la carga de cofres con inventario todavia no esta implementada");
    });

    FarmManager prepared{config, 0};
    EntityIdGenerator ids;
    if (!prepared.initialize(layout, ids)) {
        invalid("farm", "distribucion invalida: revisar entidades superpuestas, limites y tiles reservados");
    }
    return layout;
}
}

FarmLayout FarmTomlReader::read(const std::string& path, const FarmConfig& config) const {
    try {
        return readFarm(toml::parse_file(path), config);
    } catch (const toml::parse_error& error) {
        std::ostringstream message;
        message << path << ": " << error;
        throw std::invalid_argument(message.str());
    } catch (const std::invalid_argument& error) {
        throw std::invalid_argument(path + ": " + error.what());
    }
}
