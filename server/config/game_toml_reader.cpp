#include "config/game_toml_reader.h"
#include "farm/loading/farm_toml_reader.h"

#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <toml++/toml.hpp>

namespace {

[[noreturn]] void invalid(const std::string& field,
                          const std::string& reason) {
    throw std::invalid_argument(field + ": " + reason);
}

const toml::table& tableAt(const toml::node* node,
                           const std::string& field) {
    if (!node || !node->is_table()) {
        invalid(field, "se esperaba una tabla");
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

        if (root.contains("farm")) {
            world.farm = FarmTomlReader{}.read(root, world.config.farm);
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
