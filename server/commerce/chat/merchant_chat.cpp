#include "merchant_chat.h"

#include "server_constants.h"

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
        {name, server_constants::HELLO_PREFIX + context.playerName + server_constants::FIELD_USE_CATALOG_TO_SEE_MY_PRODUCTS_AND_PRICES}, {name, server_constants::COMMANDS_CATALOG_BUY_PRODUCT_AMOUNT_SELL_PRODUCT_AMOUNT}};
}

std::vector<ChatMessageData> MerchantChat::execute(const ChatCommandContext& context, const TradeCommand& command) {
    const auto player = context.players.snapshot(context.playerId);
    if (context.actionsBlocked || !player.canAct()) {
        conversations.erase(context.playerId);
        return {{server_constants::SERVER, server_constants::YOU_CANNOT_TRADE_RIGHT_NOW}};
    }
    const auto conversation = conversations.find(context.playerId);
    if (conversation == conversations.end()) {
        return {{server_constants::SERVER, server_constants::APPROACH_A_MERCHANT_AND_INTERACT_BEFORE_TRADING}};
    }
    const auto npc = world.nearbyMerchant(player);
    const auto& active = conversation->second;
    const bool sameMerchant = npc && npc->id == active.npc.id && npc->pos.getX() == active.npc.pos.getX() && npc->pos.getY() == active.npc.pos.getY();
    if (player.getInterior() != active.location || !sameMerchant) {
        conversations.erase(conversation);
        return {{server_constants::SERVER, server_constants::YOU_MOVED_AWAY_FROM_THE_MERCHANT_APPROACH_AND_INTERACT_AGAIN}};
    }
    if (command.action == TradeAction::Catalog)
        return catalog.messages(npc->id, world.farmPrices());
    if (command.quantity == 0)
        return {{server_constants::SERVER, server_constants::THE_AMOUNT_MUST_BE_GREATER_THAN_ZERO}};
    const auto result = command.action == TradeAction::Buy ? trade.buy(context.players, context.playerId, npc->id, command) :
        trade.sell(context.players, context.playerId, npc->id, command);
    return {{std::string{shop_products::merchantName(npc->id)}, tradeMessage(command, result)}};
}

std::string MerchantChat::tradeMessage(const TradeCommand& command, const TradeResult& result) {
    const bool buying = command.action == TradeAction::Buy;
    switch (result.status) {
        case TradeStatus::Completed:
            return std::string(buying ? server_constants::YOU_BOUGHT_PREFIX : server_constants::YOU_SOLD_PREFIX) + std::to_string(command.quantity) + server_constants::SPACE + command.product + server_constants::FOR_SEPARATOR + std::to_string(result.gold) + server_constants::GOLD_SUFFIX;
        case TradeStatus::UnknownAnimal:
            return buying ? server_constants::I_DO_NOT_SELL_THAT_ANIMAL : server_constants::I_DO_NOT_BUY_THAT_ANIMAL;
        case TradeStatus::UnavailableProduct:
            return buying ? server_constants::I_DO_NOT_SELL_THAT_PRODUCT_CHECK_MY_CATALOG : server_constants::I_DO_NOT_BUY_THAT_PRODUCT;
        case TradeStatus::InsufficientGold:
            return server_constants::YOU_DO_NOT_HAVE_ENOUGH_GOLD;
        case TradeStatus::FarmFull:
            return server_constants::THERE_IS_NO_ROOM_ON_THE_FARM_OR_THE_ANIMAL_LIMIT_WOULD;
        case TradeStatus::InventoryFull:
            return server_constants::THERE_IS_NOT_ENOUGH_INVENTORY_SPACE;
        case TradeStatus::GoldLimit:
            return server_constants::THE_SALE_WOULD_EXCEED_THE_GOLD_LIMIT;
        case TradeStatus::MissingAnimals:
            return server_constants::THERE_ARE_NOT_ENOUGH_ANIMALS_ON_THE_FARM;
        case TradeStatus::UnknownQuality:
            return server_constants::THE_PRODUCT_HAS_AN_UNKNOWN_QUALITY;
        case TradeStatus::MissingItems:
            return server_constants::YOU_DO_NOT_HAVE_ENOUGH_OF_THAT_PRODUCT;
        case TradeStatus::Rejected:
            return server_constants::COULD_NOT_COMPLETE_THE_SALE;
    }
    return server_constants::COULD_NOT_COMPLETE_THE_SALE;
}
