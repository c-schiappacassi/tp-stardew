#include "commerce_manager.h"

CommerceManager::CommerceManager(WorldManager& world, const MarketConfig& config, uint32_t seed):
    catalog(config, prices, seed), trade(world, prices, catalog), chat(world, catalog, trade) {}

std::optional<std::vector<ChatMessageData>> CommerceManager::interact(const ChatCommandContext& context, Direction direction) {
    return chat.interact(context, direction);
}

std::vector<ChatMessageData> CommerceManager::execute(const ChatCommandContext& context, const TradeCommand& command) {
    return chat.execute(context, command);
}

void CommerceManager::onDisconnect(uint16_t playerId) noexcept { chat.onDisconnect(playerId); }

void CommerceManager::finishDay() {
    prices.finishDay();
    chat.clearConversations();
    catalog.advanceDay();
}

void CommerceManager::scheduleDiscount() { prices.scheduleDiscount(); }

MarketDiscount CommerceManager::discountState() const { return prices.discountState(); }

void CommerceManager::restoreDiscount(MarketDiscount saved) { prices.restoreDiscount(saved); }
