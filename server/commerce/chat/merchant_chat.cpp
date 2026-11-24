#include "merchant_chat.h"

#include "commerce/catalog/merchant_catalog.h"
#include "commerce/shop_products.h"
#include "commerce/trading/merchant_trade.h"
#include "player/player_manager.h"
#include "world/world_manager.h"

MerchantChat::MerchantChat(WorldManager& world, const MerchantCatalog& catalog, const MerchantTrade& trade): world(world), catalog(catalog), trade(trade) {}

void MerchantChat::clearConversations() noexcept { conversations.clear(); }

void MerchantChat::onDisconnect(uint16_t playerId) noexcept { conversations.erase(playerId); }

std::optional<std::vector<ChatMessageData>> MerchantChat::interact(const ChatCommandContext& context, Direction direction) {
    const auto player = context.players.snapshot(context.playerId);
    if (context.actionsBlocked || !player.canAct())
        return std::nullopt;
    const auto npc = world.nearbyMerchant(player, direction);
    if (!npc)
        return std::nullopt;
    conversations.insert_or_assign(context.playerId, Conversation{*player.getInterior(), *npc});
    const std::string name{shop_products::merchantName(npc->id)};
    context.players.apply(context.playerId, player_action::FaceTarget{npc->pos});
    return std::vector<ChatMessageData>{
        {name, "Hello, " + context.playerName + ". Use /catalog to see my products and prices."}, {name, "Commands: /catalog | /buy product amount | /sell product amount"}};
}

std::vector<ChatMessageData> MerchantChat::execute(const ChatCommandContext& context, const TradeCommand& command) {
    const auto player = context.players.snapshot(context.playerId);
    if (context.actionsBlocked || !player.canAct()) {
        conversations.erase(context.playerId);
        return {{"Server", "You cannot trade right now."}};
    }
    const auto conversation = conversations.find(context.playerId);
    if (conversation == conversations.end()) {
        return {{"Server", "Approach a merchant and interact before trading."}};
    }
    const auto npc = world.nearbyMerchant(player);
    const auto& active = conversation->second;
    const bool sameMerchant = npc && npc->id == active.npc.id && npc->pos.getX() == active.npc.pos.getX() && npc->pos.getY() == active.npc.pos.getY();
    if (player.getInterior() != active.location || !sameMerchant) {
        conversations.erase(conversation);
        return {{"Server", "You moved away from the merchant. Approach and interact again."}};
    }
    if (command.action == TradeAction::Catalog)
        return catalog.messages(npc->id, world.farmPrices());
    if (command.quantity == 0)
        return {{"Server", "The amount must be greater than zero."}};
    const auto result = command.action == TradeAction::Buy ? trade.buy(context.players, context.playerId, npc->id, command) :
        trade.sell(context.players, context.playerId, npc->id, command);
    return {{std::string{shop_products::merchantName(npc->id)}, tradeMessage(command, result)}};
}

std::string MerchantChat::tradeMessage(const TradeCommand& command, const TradeResult& result) {
    const bool buying = command.action == TradeAction::Buy;
    switch (result.status) {
        case TradeStatus::Completed:
            return std::string(buying ? "You bought " : "You sold ") + std::to_string(command.quantity) + " " + command.product + " for " + std::to_string(result.gold) + " gold.";
        case TradeStatus::UnknownAnimal:
            return buying ? "I do not sell that animal." : "I do not buy that animal.";
        case TradeStatus::UnavailableProduct:
            return buying ? "I do not sell that product. Check my catalog." : "I do not buy that product.";
        case TradeStatus::InsufficientGold:
            return "You do not have enough gold.";
        case TradeStatus::FarmFull:
            return "There is no room on the farm or the animal limit would be exceeded.";
        case TradeStatus::InventoryFull:
            return "There is not enough inventory space.";
        case TradeStatus::GoldLimit:
            return "The sale would exceed the gold limit.";
        case TradeStatus::MissingAnimals:
            return "There are not enough animals on the farm.";
        case TradeStatus::UnknownQuality:
            return "The product has an unknown quality.";
        case TradeStatus::MissingItems:
            return "You do not have enough of that product.";
        case TradeStatus::Rejected:
            return "Could not complete the sale.";
    }
    return "Could not complete the sale.";
}
