#pragma once

#include <cstdint>

#include "chat/commands/trade_command.h"
#include "protocol_types.h"

class MarketPrices;
class MerchantCatalog;
class PlayerManager;
class WorldManager;

enum class TradeStatus {
    Completed,
    UnknownAnimal,
    UnavailableProduct,
    InsufficientGold,
    FarmFull,
    InventoryFull,
    GoldLimit,
    MissingAnimals,
    UnknownQuality,
    MissingItems,
    Rejected
};

struct TradeResult {
    TradeStatus status;
    uint32_t gold = 0;
};

class MerchantTrade {
    WorldManager& world;
    const MarketPrices& prices;
    const MerchantCatalog& catalog;

public:
    MerchantTrade(WorldManager& world, const MarketPrices& prices, const MerchantCatalog& catalog);

    TradeResult buy(PlayerManager& players, uint16_t playerId, NpcId merchant, const TradeCommand& command) const;
    TradeResult sell(PlayerManager& players, uint16_t playerId, NpcId merchant, const TradeCommand& command) const;
};
