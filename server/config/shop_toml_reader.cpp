#include "config/shop_toml_reader.h"

#include "server_constants.h"

#include <cstddef>
#include <limits>
#include <set>
#include <string>

#include "config/toml_helpers.h"
#include "farm/decorations/decoration_items.h"

ItemId ShopTomlReader::itemFromId(uint16_t id, const std::string& field) {
    const auto item = static_cast<ItemId>(id);
    if (item == ItemId::Chest || decoration_items::isDecoration(item)) return item;
    toml_helpers::invalid(field, server_constants::UNKNOWN_PRODUCT_ID_OR_FIXED_FURNITURE);
}

RobinShopConfig ShopTomlReader::read(const toml::table& root) const {
    using toml_helpers::boolean;
    using toml_helpers::integer;
    using toml_helpers::invalid;
    using toml_helpers::tableAt;

    const auto& shops = tableAt(root.get(server_constants::SHOPS), server_constants::SHOPS);
    const auto& robin = tableAt(shops.get(server_constants::ROBIN_2), server_constants::SHOPS_ROBIN);

    RobinShopConfig config;
    config.products.clear();
    config.dailyRotationCount = static_cast<uint8_t>(integer(
        robin, server_constants::DAILY_ROTATION_COUNT, std::numeric_limits<uint8_t>::max(),
        server_constants::SHOPS_ROBIN_DAILY_ROTATION_COUNT));

    const auto* node = robin.get(server_constants::PRODUCTS);
    if (!node || !node->is_array()) {
        invalid(server_constants::SHOPS_ROBIN_PRODUCTS, server_constants::EXPECTED_AN_ARRAY);
    }

    const auto& products = *node->as_array();
    std::set<ItemId> registered;
    std::size_t rotatingProducts = 0;
    config.products.reserve(products.size());

    for (std::size_t i = 0; i < products.size(); ++i) {
        const auto field = server_constants::SHOPS_ROBIN_PRODUCTS_PREFIX + std::to_string(i) + server_constants::CLOSE_BRACKET;
        const auto& product = tableAt(&products[i], field);

        const auto id = static_cast<uint16_t>(integer(
            product, server_constants::ITEM_ID, std::numeric_limits<uint16_t>::max(),
            field + server_constants::FIELD_ITEM_ID));
        const ItemId item = itemFromId(id, field + server_constants::FIELD_ITEM_ID);

        const auto price = static_cast<uint32_t>(integer(
            product, server_constants::PRICE, std::numeric_limits<uint32_t>::max(),
            field + server_constants::FIELD_PRICE));

        const bool rotates = boolean(product, server_constants::ROTATES, field + server_constants::FIELD_ROTATES);

        if (!registered.insert(item).second) {
            invalid(field, server_constants::DUPLICATE_PRODUCT);
        }

        if (item == ItemId::Chest && rotates) {
            invalid(field, server_constants::THE_CHEST_CANNOT_ROTATE);
        }

        if (item != ItemId::Chest && !rotates) {
            invalid(field, server_constants::DECORATIONS_MUST_ROTATE);
        }

        if (rotates) {
            ++rotatingProducts;
        }

        config.products.push_back({item, price, rotates});
    }

    if (config.dailyRotationCount > rotatingProducts) {
        invalid(server_constants::SHOPS_ROBIN_DAILY_ROTATION_COUNT,
                server_constants::CANNOT_EXCEED_THE_NUMBER_OF_ROTATING_PRODUCTS);
    }

    return config;
}

MarketConfig ShopTomlReader::readMarket(const toml::table& root) const {
    MarketConfig config;
    config.robin = read(root);
    const auto readPrices = [&](const char* shop, std::vector<ShopItemPrice>& prices) {
        const auto& shops = toml_helpers::tableAt(root.get(server_constants::SHOPS), server_constants::SHOPS);
        const auto* shopNode = shops.get(shop);
        if (!shopNode) return;
        const std::string field = std::string(server_constants::SHOPS_2) + shop + server_constants::FIELD_PRODUCTS;
        const auto& shopTable = toml_helpers::tableAt(shopNode, std::string(server_constants::SHOPS_2) + shop);
        const auto* node = shopTable.get(server_constants::PRODUCTS);
        if (!node || !node->is_array()) toml_helpers::invalid(field, server_constants::EXPECTED_AN_ARRAY);
        std::set<ItemId> registered;
        for (const auto& entry : *node->as_array()) {
            const auto& product = toml_helpers::tableAt(&entry, field);
            const auto item = static_cast<ItemId>(toml_helpers::integer(product, server_constants::ITEM_ID,
                std::numeric_limits<uint16_t>::max(), field + server_constants::FIELD_ITEM_ID));
            const auto price = static_cast<uint32_t>(toml_helpers::integer(product, server_constants::PRICE,
                std::numeric_limits<uint32_t>::max(), field + server_constants::FIELD_PRICE));
            if (!registered.insert(item).second) toml_helpers::invalid(field, server_constants::DUPLICATE_PRODUCT);
            bool found = false;
            for (auto& configured : prices) {
                if (configured.item == item) {
                    configured.price = price;
                    found = true;
                }
            }
            if (!found) toml_helpers::invalid(field, server_constants::PRODUCT_NOT_SUPPORTED_BY_THIS_MERCHANT);
        }
    };
    readPrices(server_constants::PIERRE_2, config.seeds);
    readPrices(server_constants::CLINT_2, config.minerals);
    return config;
}
