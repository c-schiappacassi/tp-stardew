#include <algorithm>
#include <limits>

#include "player/player_manager.h"
#include "world/world_manager.h"

#include "merchant_chat.h"
#include "shop_products.h"

namespace {
std::optional<uint32_t> priceOf(const std::vector<ShopItemPrice>& prices, ItemId item) {
    for (const auto& entry: prices) {
        if (entry.item == item)
            return entry.price;
    }
    return std::nullopt;
}

std::string success(const TradeCommand& command, uint32_t price) {
    return std::string(command.action == TradeAction::Buy ? "You bought " : "You sold ") + std::to_string(command.quantity) + " " + command.product + " for " +
        std::to_string(price) + " gold.";
}
}  // namespace

std::string MerchantChat::buy(PlayerManager& players, uint16_t playerId, NpcId merchant, const TradeCommand& command) {
    const auto player = players.snapshot(playerId);
    if (merchant == NpcId::Marnie) {
        const auto species = shop_products::animal(command.product);
        const auto basePrice = species ? world.getFarm().animalBasePrice(*species) : std::nullopt;
        if (!basePrice)
            return "I do not sell that animal.";
        const uint64_t total = prices.buyingPrice(*basePrice) * command.quantity;
        if (total > player.getGold())
            return "You do not have enough gold.";
        if (!world.buyAnimals(*species, command.quantity))
            return "There is no room on the farm or the animal limit would be exceeded.";
        players.apply(playerId, player_action::SpendGold{static_cast<uint32_t>(total)});
        return success(command, static_cast<uint32_t>(total));
    }
    const auto item = shop_products::item(command.product);
    std::optional<uint32_t> unitPrice;
    if (item && merchant == NpcId::Pierre)
        unitPrice = priceOf(config.seeds, *item);
    if (item && merchant == NpcId::Robin) {
        for (const auto& product: robinCatalog) {
            if (product.item == *item)
                unitPrice = product.price;
        }
    }
    if (!unitPrice)
        return "I do not sell that product. Check my catalog.";
    const uint64_t total = prices.buyingPrice(*unitPrice) * command.quantity;
    if (total > player.getGold())
        return "You do not have enough gold.";
    if (!players.apply(playerId, player_action::BuyItem{{*item, command.quantity, Quality::None}, static_cast<uint32_t>(total)}).applied) {
        return "There is not enough inventory space.";
    }
    return success(command, static_cast<uint32_t>(total));
}

std::string MerchantChat::sell(PlayerManager& players, uint16_t playerId, NpcId merchant, const TradeCommand& command) {
    const auto player = players.snapshot(playerId);
    const auto items = players.inventory(playerId);
    const uint64_t goldSpace = std::numeric_limits<uint32_t>::max() - player.getGold();
    if (merchant == NpcId::Marnie) {
        const auto species = shop_products::animal(command.product);
        const auto basePrice = species ? world.getFarm().animalBasePrice(*species) : std::nullopt;
        if (!basePrice)
            return "I do not buy that animal.";
        const uint64_t total = prices.sellingPrice(*basePrice) * command.quantity;
        if (total > goldSpace)
            return "The sale would exceed the gold limit.";
        if (!world.sellAnimals(*species, command.quantity))
            return "There are not enough animals on the farm.";
        players.apply(playerId, player_action::AddGold{static_cast<uint32_t>(total)});
        return success(command, static_cast<uint32_t>(total));
    }
    const auto item = shop_products::item(command.product);
    std::optional<uint32_t> basePrice;
    if (item && merchant == NpcId::Clint)
        basePrice = priceOf(config.minerals, *item);
    if (item && merchant == NpcId::Pierre)
        basePrice = world.getFarm().productBasePrice(*item);
    if (!basePrice)
        return "I do not buy that product.";
    std::vector<std::pair<uint8_t, uint16_t>> quantities;
    uint16_t remaining = command.quantity;
    uint64_t totalQuarters = 0;
    for (const auto& [slot, stack]: items) {
        if (stack.item_id != *item || remaining == 0)
            continue;
        const auto unitPrice = shop_products::salePriceQuarters(*basePrice, stack.qual);
        if (!unitPrice)
            return "The product has an unknown quality.";
        const auto quantity = std::min(remaining, stack.quantity);
        totalQuarters += prices.sellingPrice(*unitPrice) * quantity;
        quantities.emplace_back(slot, quantity);
        remaining -= quantity;
    }
    if (remaining != 0)
        return "You do not have enough of that product.";
    const uint64_t total = totalQuarters / 4;
    if (total > goldSpace)
        return "The sale would exceed the gold limit.";
    if (!players.apply(playerId, player_action::SellItems{quantities, static_cast<uint32_t>(total)}).applied)
        return "Could not complete the sale.";
    return success(command, static_cast<uint32_t>(total));
}
