#include "merchant_catalog.h"

#include <algorithm>
#include <set>
#include <stdexcept>

#include "commerce/market_prices.h"
#include "commerce/shop_products.h"
#include "farm/decorations/decoration_items.h"
#include "farm/queries/farm_prices.h"

namespace {
std::optional<uint32_t> priceOf(const std::vector<ShopItemPrice>& prices, ItemId item) {
    for (const auto& entry: prices) {
        if (entry.item == item)
            return entry.price;
    }
    return std::nullopt;
}
}

MerchantCatalog::MerchantCatalog(const MarketConfig& config, const MarketPrices& prices, uint32_t seed): config(config), prices(prices), random(seed) {
    std::set<ItemId> registered;
    std::size_t rotating = 0;
    for (const auto& product: config.robin.products) {
        if (!registered.insert(product.item).second ||
            (product.item != ItemId::Chest && !decoration_items::isDecoration(product.item)) ||
            product.rotates != (product.item != ItemId::Chest)) {
            throw std::invalid_argument("Invalid Robin catalog");
        }
        if (product.rotates)
            ++rotating;
    }
    if (config.robin.dailyRotationCount > rotating) {
        throw std::invalid_argument("Robin rotation exceeds the catalog size");
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
    std::vector<ChatMessageData> messages{{name, "Catalog (unit prices in gold):"}};
    if (prices.hasDiscount())
        messages.push_back({name, "Community reward active today: buy at half price and sell for double."});
    const auto add = [&](std::string_view product, uint32_t price, bool buys) {
        messages.push_back({name, std::string(buys ? "Buying " : "Selling ") + std::string(product) +
            ": " + std::to_string(buys ? prices.sellingPrice(price) : prices.buyingPrice(price))});
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
            const auto product = animal.species == AnimalSpecies::Cow ? "cow" : "chicken";
            add(product, animal.basePrice, false);
            add(product, animal.basePrice, true);
        }
    }
    if (merchant == NpcId::Pierre || merchant == NpcId::Clint) {
        messages.push_back({name, "Buying prices are for base quality. Item quality: good x1.25, excellent x2, incredible x4, perfect x8."});
    }
    return messages;
}
