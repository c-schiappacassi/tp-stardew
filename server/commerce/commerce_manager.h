#pragma once

#include <cstdint>
#include <random>

#include "catalog/merchant_catalog.h"
#include "chat/merchant_chat.h"
#include "chat/services/merchant_chat_service.h"
#include "trading/merchant_trade.h"

#include "market_prices.h"
#include "shop_config.h"

class WorldManager;

class CommerceManager final: public MerchantChatService {
    MarketPrices prices;
    MerchantCatalog catalog;
    MerchantTrade trade;
    MerchantChat chat;

public:
    explicit CommerceManager(WorldManager& world, const MarketConfig& config = {}, uint32_t seed = std::random_device{}());

    CommerceManager(const CommerceManager&) = delete;
    CommerceManager& operator=(const CommerceManager&) = delete;
    CommerceManager(CommerceManager&&) = delete;
    CommerceManager& operator=(CommerceManager&&) = delete;

    std::optional<std::vector<ChatMessageData>> interact(const ChatCommandContext& context, Direction direction) override;
    std::vector<ChatMessageData> execute(const ChatCommandContext& context, const TradeCommand& command) override;
    void onDisconnect(uint16_t playerId) noexcept override;
    void finishDay();
    void scheduleDiscount();
    MarketDiscount discountState() const;
    void restoreDiscount(MarketDiscount saved);
};
