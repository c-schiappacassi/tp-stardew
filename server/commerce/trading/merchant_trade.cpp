#include "merchant_trade.h"

#include <algorithm>
#include <limits>

#include "commerce/catalog/merchant_catalog.h"
#include "commerce/market_prices.h"
#include "commerce/shop_products.h"
#include "player/player_manager.h"
#include "world/world_manager.h"

MerchantTrade::MerchantTrade(WorldManager& world, const MarketPrices& prices, const MerchantCatalog& catalog): world(world), prices(prices), catalog(catalog) {}

TradeResult MerchantTrade::buy(PlayerManager& players, uint16_t playerId, NpcId merchant, const TradeCommand& command) const {
    const auto player = players.snapshot(playerId);
    if (merchant == NpcId::Marnie) {
        const auto species = shop_products::animal(command.product);
        const auto basePrice = species ? world.animalBasePrice(*species) : std::nullopt;
        if (!basePrice)
            return {TradeStatus::UnknownAnimal};
        const uint64_t total = prices.buyingPrice(*basePrice) * command.quantity;
        if (total > player.getGold())
            return {TradeStatus::InsufficientGold};
        if (!world.buyAnimals(*species, command.quantity))
            return {TradeStatus::FarmFull};
        players.apply(playerId, player_action::SpendGold{static_cast<uint32_t>(total)});
        return {TradeStatus::Completed, static_cast<uint32_t>(total)};
    }
    const auto item = shop_products::item(command.product);
    const auto unitPrice = item ? catalog.buyingBasePrice(merchant, *item) : std::nullopt;
    if (!unitPrice)
        return {TradeStatus::UnavailableProduct};
    const uint64_t total = prices.buyingPrice(*unitPrice) * command.quantity;
    if (total > player.getGold())
        return {TradeStatus::InsufficientGold};
    if (!players.apply(playerId, player_action::BuyItem{{*item, command.quantity, Quality::None}, static_cast<uint32_t>(total)}).applied) {
        return {TradeStatus::InventoryFull};
    }
    return {TradeStatus::Completed, static_cast<uint32_t>(total)};
}

TradeResult MerchantTrade::sell(PlayerManager& players, uint16_t playerId, NpcId merchant, const TradeCommand& command) const {
    const auto player = players.snapshot(playerId);
    const uint64_t goldSpace = std::numeric_limits<uint32_t>::max() - player.getGold();
    if (merchant == NpcId::Marnie) {
        const auto species = shop_products::animal(command.product);
        const auto basePrice = species ? world.animalBasePrice(*species) : std::nullopt;
        if (!basePrice)
            return {TradeStatus::UnknownAnimal};
        const uint64_t total = prices.sellingPrice(*basePrice) * command.quantity;
        if (total > goldSpace)
            return {TradeStatus::GoldLimit};
        if (!world.sellAnimals(*species, command.quantity))
            return {TradeStatus::MissingAnimals};
        players.apply(playerId, player_action::AddGold{static_cast<uint32_t>(total)});
        return {TradeStatus::Completed, static_cast<uint32_t>(total)};
    }
    const auto item = shop_products::item(command.product);
    std::optional<uint32_t> basePrice;
    if (item && merchant == NpcId::Clint)
        basePrice = catalog.mineralBasePrice(*item);
    if (item && merchant == NpcId::Pierre)
        basePrice = world.productBasePrice(*item);
    if (!basePrice)
        return {TradeStatus::UnavailableProduct};
    const auto items = players.inventory(playerId);
    std::vector<std::pair<uint8_t, uint16_t>> quantities;
    uint16_t remaining = command.quantity;
    uint64_t totalQuarters = 0;
    for (const auto& [slot, stack]: items) {
        if (stack.item_id != *item || remaining == 0)
            continue;
        const auto unitPrice = shop_products::salePriceQuarters(*basePrice, stack.qual);
        if (!unitPrice)
            return {TradeStatus::UnknownQuality};
        const auto quantity = std::min(remaining, stack.quantity);
        totalQuarters += prices.sellingPrice(*unitPrice) * quantity;
        quantities.emplace_back(slot, quantity);
        remaining -= quantity;
    }
    if (remaining != 0)
        return {TradeStatus::MissingItems};
    const uint64_t total = totalQuarters / 4;
    if (total > goldSpace)
        return {TradeStatus::GoldLimit};
    if (!players.apply(playerId, player_action::SellItems{quantities, static_cast<uint32_t>(total)}).applied)
        return {TradeStatus::Rejected};
    return {TradeStatus::Completed, static_cast<uint32_t>(total)};
}
