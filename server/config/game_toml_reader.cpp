#include "config/game_toml_reader.h"

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

GameConfig GameTomlReader::read(const std::string& path) const {
    try {
        const auto root = toml::parse_file(path);

        GameConfig config;

        readDeath(root, config);
        readRobin(root, config);

        return config;

    } catch (const toml::parse_error& error) {
        std::ostringstream message;
        message << path << ": " << error;
        throw std::invalid_argument(message.str());

    } catch (const std::invalid_argument& error) {
        throw std::invalid_argument(path + ": " + error.what());
    }
}
