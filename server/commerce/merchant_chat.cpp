#include "merchant_chat.h"

#include "player/player.h"
#include "shop_products.h"
#include "world/world_manager.h"

void MerchantChat::finishDay() {
    conversations.clear();
    rotateCatalog();
}

void MerchantChat::onDisconnect(uint16_t playerId) noexcept { conversations.erase(playerId); }

std::optional<std::vector<ChatMessageData>> MerchantChat::interact(
    const ChatCommandContext& context, Direction direction) {
    if (context.actionsBlocked || !context.player.canAct()) return std::nullopt;
    const auto npc = world.nearbyMerchant(context.player, direction);
    if (!npc) return std::nullopt;
    conversations.insert_or_assign(context.playerId, Conversation{*context.player.getInterior(), *npc});
    const std::string name{shop_products::merchantName(npc->id)};
    context.player.faceTarget(npc->pos);
    return std::vector<ChatMessageData>{
        {name, "Hello, " + context.playerName + ". Use /catalog to see my products and prices."},
        {name, "Commands: /catalog | /buy product amount | /sell product amount"}
    };
}

std::vector<ChatMessageData> MerchantChat::execute(
    const ChatCommandContext& context, const TradeCommand& command) {
    if (context.actionsBlocked || !context.player.canAct()) {
        conversations.erase(context.playerId);
        return {{"Server", "You cannot trade right now."}};
    }
    const auto conversation = conversations.find(context.playerId);
    if (conversation == conversations.end()) {
        return {{"Server", "Approach a merchant and interact before trading."}};
    }
    const auto npc = world.nearbyMerchant(context.player);
    const auto& active = conversation->second;
    if (context.player.getInterior() != active.location || !npc || npc->id != active.npc.id ||
        npc->pos.getX() != active.npc.pos.getX() || npc->pos.getY() != active.npc.pos.getY()) {
        conversations.erase(conversation);
        return {{"Server", "You moved away from the merchant. Approach and interact again."}};
    }
    if (command.action == TradeAction::Catalog) return catalog(npc->id);
    if (command.quantity == 0) return {{"Server", "The amount must be greater than zero."}};
    const auto response = command.action == TradeAction::Buy
        ? buy(context.player, npc->id, command) : sell(context.player, npc->id, command);
    return {{std::string{shop_products::merchantName(npc->id)}, response}};
}
