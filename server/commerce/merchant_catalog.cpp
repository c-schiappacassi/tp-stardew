#include "merchant_chat.h"

#include <set>
#include <stdexcept>

#include "farm/decorations/decoration.h"
#include "shop_products.h"
#include "world/world_manager.h"

MerchantChat::MerchantChat(WorldManager& world, const MarketPrices& prices, const MarketConfig& config) :
    world(world), prices(prices), config(config) {
    std::set<ItemId> registered;
    std::size_t rotating = 0;
    for (const auto& product : config.robin.products) {
        if (!registered.insert(product.item).second ||
            (product.item != ItemId::Chest && !Decoration::isDecoration(product.item)) ||
            product.rotates != (product.item != ItemId::Chest)) {
            throw std::invalid_argument("Invalid Robin catalog");
        }
        if (product.rotates) ++rotating;
    }
    if (config.robin.dailyRotationCount > rotating) {
        throw std::invalid_argument("Robin rotation exceeds the catalog size");
    }
    rotateCatalog();
}

void MerchantChat::rotateCatalog() {
    robinCatalog.clear();
    std::vector<RobinProductConfig> rotating;
    for (const auto& product : config.robin.products) {
        if (product.rotates) rotating.push_back(product);
        else robinCatalog.push_back(product);
    }
    for (std::size_t i = 0; i < config.robin.dailyRotationCount; ++i) {
        robinCatalog.push_back(rotating[(rotation + i) % rotating.size()]);
    }
    if (!rotating.empty()) rotation = (rotation + config.robin.dailyRotationCount) % rotating.size();
}

std::vector<ChatMessageData> MerchantChat::catalog(NpcId merchant) const {
    const std::string name{shop_products::merchantName(merchant)};
    std::vector<ChatMessageData> messages{{name, "Catalog (unit prices in gold):"}};
    if (prices.hasDiscount()) messages.push_back({name, "Community reward active today: buy at half price and sell for double."});
    const auto add = [&](std::string_view product, uint32_t price, bool buys) {
        messages.push_back({name, std::string(buys ? "Buying " : "Selling ") + std::string(product) +
                                  ": " + std::to_string(buys ? prices.sellingPrice(price) : prices.buyingPrice(price))});
    };
    const auto& farmConfig = world.getFarm().getConfig();
    if (merchant == NpcId::Robin) {
        for (const auto& entry : robinCatalog) add(shop_products::name(entry.item), entry.price, false);
    } else if (merchant == NpcId::Clint) {
        for (const auto& entry : config.minerals) add(shop_products::name(entry.item), entry.price, true);
    } else if (merchant == NpcId::Pierre) {
        for (const auto& entry : config.seeds) add(shop_products::name(entry.item), entry.price, false);
        for (const auto& crop : farmConfig.crops) {
            const auto item = shop_products::cropItem(crop.type);
            if (item) add(shop_products::name(*item), crop.basePrice, true);
        }
        for (const auto& animal : farmConfig.animals) {
            add(animal.species == AnimalSpecies::Cow ? "milk" : "egg", animal.productPrice, true);
        }
        messages.push_back({name, "Crops: base quality x1, good x1.25, excellent x2, incredible x4, perfect x8."});
    } else if (merchant == NpcId::Marnie) {
        for (const auto& animal : farmConfig.animals) {
            const auto product = animal.species == AnimalSpecies::Cow ? "cow" : "chicken";
            add(product, animal.basePrice, false);
            add(product, animal.basePrice, true);
        }
    }
    return messages;
}
