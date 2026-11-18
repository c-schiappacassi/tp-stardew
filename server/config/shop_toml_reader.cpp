#include "config/shop_toml_reader.h"

#include <cstddef>
#include <limits>
#include <set>
#include <string>

#include "config/toml_helpers.h"
#include "farm/decorations/decoration.h"

ItemId ShopTomlReader::itemFromId(uint16_t id, const std::string& field) {
    const auto item = static_cast<ItemId>(id);
    if (item == ItemId::Chest || Decoration::isDecoration(item)) return item;
    toml_helpers::invalid(field, "ID de producto desconocido o mueble fijo");
}

RobinShopConfig ShopTomlReader::read(const toml::table& root) const {
    using toml_helpers::boolean;
    using toml_helpers::integer;
    using toml_helpers::invalid;
    using toml_helpers::tableAt;

    const auto& shops = tableAt(root.get("shops"), "shops");
    const auto& robin = tableAt(shops.get("robin"), "shops.robin");

    RobinShopConfig config;
    config.products.clear();
    config.dailyRotationCount = static_cast<uint8_t>(integer(
        robin, "daily_rotation_count", std::numeric_limits<uint8_t>::max(),
        "shops.robin.daily_rotation_count"));

    const auto* node = robin.get("products");
    if (!node || !node->is_array()) {
        invalid("shops.robin.products", "se esperaba una lista");
    }

    const auto& products = *node->as_array();
    std::set<ItemId> registered;
    std::size_t rotatingProducts = 0;
    config.products.reserve(products.size());

    for (std::size_t i = 0; i < products.size(); ++i) {
        const auto field = "shops.robin.products[" + std::to_string(i) + "]";
        const auto& product = tableAt(&products[i], field);

        const auto id = static_cast<uint16_t>(integer(
            product, "item_id", std::numeric_limits<uint16_t>::max(),
            field + ".item_id"));
        const ItemId item = itemFromId(id, field + ".item_id");

        const auto price = static_cast<uint32_t>(integer(
            product, "price", std::numeric_limits<uint32_t>::max(),
            field + ".price"));

        const bool rotates = boolean(product, "rotates", field + ".rotates");

        if (!registered.insert(item).second) {
            invalid(field, "producto repetido");
        }

        if (item == ItemId::Chest && rotates) {
            invalid(field, "el cofre no puede rotar");
        }

        if (item != ItemId::Chest && !rotates) {
            invalid(field, "las decoraciones deben rotar");
        }

        if (rotates) {
            ++rotatingProducts;
        }

        config.products.push_back({item, price, rotates});
    }

    if (config.dailyRotationCount > rotatingProducts) {
        invalid("shops.robin.daily_rotation_count",
                "no puede superar la cantidad de productos que rotan");
    }

    return config;
}

MarketConfig ShopTomlReader::readMarket(const toml::table& root) const {
    MarketConfig config;
    config.robin = read(root);
    const auto readPrices = [&](const char* shop, std::vector<ShopItemPrice>& prices) {
        const auto& shops = toml_helpers::tableAt(root.get("shops"), "shops");
        const auto* shopNode = shops.get(shop);
        if (!shopNode) return;
        const std::string field = std::string("shops.") + shop + ".products";
        const auto& shopTable = toml_helpers::tableAt(shopNode, std::string("shops.") + shop);
        const auto* node = shopTable.get("products");
        if (!node || !node->is_array()) toml_helpers::invalid(field, "se esperaba una lista");
        std::set<ItemId> registered;
        for (const auto& entry : *node->as_array()) {
            const auto& product = toml_helpers::tableAt(&entry, field);
            const auto item = static_cast<ItemId>(toml_helpers::integer(product, "item_id",
                std::numeric_limits<uint16_t>::max(), field + ".item_id"));
            const auto price = static_cast<uint32_t>(toml_helpers::integer(product, "price",
                std::numeric_limits<uint32_t>::max(), field + ".price"));
            if (!registered.insert(item).second) toml_helpers::invalid(field, "producto repetido");
            bool found = false;
            for (auto& configured : prices) {
                if (configured.item == item) {
                    configured.price = price;
                    found = true;
                }
            }
            if (!found) toml_helpers::invalid(field, "producto no admitido por este comerciante");
        }
    };
    readPrices("pierre", config.seeds);
    readPrices("clint", config.minerals);
    return config;
}
