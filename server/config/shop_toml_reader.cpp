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
    toml_helpers::invalid(field, "unknown product ID or fixed furniture");
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
        invalid("shops.robin.products", "expected an array");
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
            invalid(field, "duplicate product");
        }

        if (item == ItemId::Chest && rotates) {
            invalid(field, "the chest cannot rotate");
        }

        if (item != ItemId::Chest && !rotates) {
            invalid(field, "decorations must rotate");
        }

        if (rotates) {
            ++rotatingProducts;
        }

        config.products.push_back({item, price, rotates});
    }

    if (config.dailyRotationCount > rotatingProducts) {
        invalid("shops.robin.daily_rotation_count",
                "cannot exceed the number of rotating products");
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
        if (!node || !node->is_array()) toml_helpers::invalid(field, "expected an array");
        std::set<ItemId> registered;
        for (const auto& entry : *node->as_array()) {
            const auto& product = toml_helpers::tableAt(&entry, field);
            const auto item = static_cast<ItemId>(toml_helpers::integer(product, "item_id",
                std::numeric_limits<uint16_t>::max(), field + ".item_id"));
            const auto price = static_cast<uint32_t>(toml_helpers::integer(product, "price",
                std::numeric_limits<uint32_t>::max(), field + ".price"));
            if (!registered.insert(item).second) toml_helpers::invalid(field, "duplicate product");
            bool found = false;
            for (auto& configured : prices) {
                if (configured.item == item) {
                    configured.price = price;
                    found = true;
                }
            }
            if (!found) toml_helpers::invalid(field, "product not supported by this merchant");
        }
    };
    readPrices("pierre", config.seeds);
    readPrices("clint", config.minerals);
    return config;
}
