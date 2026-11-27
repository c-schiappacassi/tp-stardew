#include "merchant_catalog.h"

#include "server_constants.h"

#include <algorithm>
#include <set>
#include <stdexcept>

#include "commerce/market_prices.h"
#include "commerce/shop_products.h"
#include "farm/decorations/decoration_items.h"
#include "farm/queries/farm_prices.h"

std::optional<uint32_t> MerchantCatalog::priceOf(const std::vector<ShopItemPrice>& prices, ItemId item) {
    for (const auto& entry: prices) {
        if (entry.item == item)
            return entry.price;
    }
    return std::nullopt;
}

MerchantCatalog::MerchantCatalog(const MarketConfig& config, const MarketPrices& prices, uint32_t seed): config(config), prices(prices), random(seed) {
    std::set<ItemId> registered;
    std::size_t rotating = 0;
    for (const auto& product: config.robin.products) {
        if (!registered.insert(product.item).second ||
            (product.item != ItemId::Chest && !decoration_items::isDecoration(product.item)) ||
            product.rotates != (product.item != ItemId::Chest)) {
            throw std::invalid_argument(server_constants::INVALID_ROBIN_CATALOG);
        }
        if (product.rotates)
            ++rotating;
    }
    if (config.robin.dailyRotationCount > rotating) {
        throw std::invalid_argument(server_constants::ROBIN_ROTATION_EXCEEDS_THE_CATALOG_SIZE);
    }
    advanceDay();
}

void MerchantCatalog::advanceDay() {
    robinCatalog.clear();
    std::vector<RobinProductConfig> rotating;
    for (const auto& product: config.robin.products) {
        if (product.rotates)
            rotating.push_back(product);
        else
            robinCatalog.push_back(product);
    }
    std::shuffle(rotating.begin(), rotating.end(), random);
    robinCatalog.insert(robinCatalog.end(), rotating.begin(), rotating.begin() + config.robin.dailyRotationCount);
}

std::optional<uint32_t> MerchantCatalog::buyingBasePrice(NpcId merchant, ItemId item) const {
    if (merchant == NpcId::Pierre)
        return priceOf(config.seeds, item);
    if (merchant == NpcId::Robin) {
        for (const auto& product: robinCatalog) {
            if (product.item == item)
                return product.price;
        }
    }
    return std::nullopt;
}

std::optional<uint32_t> MerchantCatalog::mineralBasePrice(ItemId item) const { return priceOf(config.minerals, item); }

std::vector<ChatMessageData> MerchantCatalog::messages(NpcId merchant, const FarmPrices& farmPrices) const {
    const std::string name{shop_products::merchantName(merchant)};
    std::vector<ChatMessageData> messages{{name, server_constants::CATALOG_UNIT_PRICES_IN_GOLD}};
    if (prices.hasDiscount())
        messages.push_back({name, server_constants::COMMUNITY_REWARD_ACTIVE_TODAY_BUY_AT_HALF_PRICE_AND_SELL_FOR_DOUBLE});
    const auto add = [&](std::string_view product, uint32_t price, bool buys) {
        messages.push_back({name, std::string(buys ? server_constants::BUYING_PREFIX : server_constants::SELLING_PREFIX) + std::string(product) +
            server_constants::COLON_SPACE + std::to_string(buys ? prices.sellingPrice(price) : prices.buyingPrice(price))});
    };
    if (merchant == NpcId::Robin) {
        for (const auto& entry: robinCatalog) add(shop_products::name(entry.item), entry.price, false);
    } else if (merchant == NpcId::Clint) {
        for (const auto& entry: config.minerals) add(shop_products::name(entry.item), entry.price, true);
    } else if (merchant == NpcId::Pierre) {
        for (const auto& entry: config.seeds) add(shop_products::name(entry.item), entry.price, false);
        for (const auto& product: farmPrices.products)
            add(shop_products::name(product.item), product.basePrice, true);
    } else if (merchant == NpcId::Marnie) {
        for (const auto& animal: farmPrices.animals) {
            const auto product = animal.species == AnimalSpecies::Cow ? server_constants::COW : server_constants::CHICKEN;
            add(product, animal.basePrice, false);
            add(product, animal.basePrice, true);
        }
    }
    if (merchant == NpcId::Pierre || merchant == NpcId::Clint) {
        messages.push_back({name, server_constants::BUYING_PRICES_ARE_FOR_BASE_QUALITY_ITEM_QUALITY_GOOD_X1_25_EXCELLENT});
    }
    return messages;
}
