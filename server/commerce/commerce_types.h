#pragma once

#include <cstdint>
#include <vector>

#include "protocol_types.h"

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

struct RobinProductConfig {
    ItemId item;
    uint32_t price;
    bool rotates;
};

struct RobinShopConfig {
    uint8_t dailyRotationCount = 7;
    std::vector<RobinProductConfig> products{
        {ItemId::Chest, 250, false}, {ItemId::Chair, 500, true},
        {ItemId::Table, 300, true}, {ItemId::Bonsai, 350, true},
        {ItemId::BeigeRug, 200, true}, {ItemId::SimpleLamp, 400, true},
        {ItemId::BookShelf, 600, true}, {ItemId::RoundTable, 450, true},
        {ItemId::GreenStool, 150, true}
    };
};

struct ShopItemPrice {
    ItemId item;
    uint32_t price;
};

struct MarketConfig {
    RobinShopConfig robin;
    std::vector<ShopItemPrice> seeds{
        {ItemId::StrawberrySeed, 10}, {ItemId::ScallionSeed, 5},
        {ItemId::PotatoSeed, 8}, {ItemId::PumpkinSeed, 20}
    };
    std::vector<ShopItemPrice> minerals{
        {ItemId::Coal, 10}, {ItemId::Lapis, 40},
        {ItemId::Silver, 50}, {ItemId::Gold, 80}
    };
};
