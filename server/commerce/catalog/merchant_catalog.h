#pragma once

#include <optional>
#include <random>
#include <vector>

#include "commerce/shop_config.h"
#include "dtos.h"

class MarketPrices;
struct FarmPrices;

class MerchantCatalog {
    MarketConfig config;
    const MarketPrices& prices;
    std::vector<RobinProductConfig> robinCatalog;
    std::mt19937 random;

public:
    MerchantCatalog(const MarketConfig& config, const MarketPrices& prices, uint32_t seed);

    void advanceDay();
    std::optional<uint32_t> buyingBasePrice(NpcId merchant, ItemId item) const;
    std::optional<uint32_t> mineralBasePrice(ItemId item) const;
    std::vector<ChatMessageData> messages(NpcId merchant, const FarmPrices& farmPrices) const;
};
