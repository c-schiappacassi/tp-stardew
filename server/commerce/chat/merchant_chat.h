#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "chat/chat_command_context.h"
#include "chat/commands/trade_command.h"
#include "dtos.h"

#include "commerce/commerce_types.h"

class MerchantCatalog;
class MerchantTrade;
class WorldManager;

class MerchantChat {
    struct Conversation {
        InteriorLocation location;
        NpcData npc;
    };

    WorldManager& world;
    const MerchantCatalog& catalog;
    const MerchantTrade& trade;
    std::map<uint16_t, Conversation> conversations;

    static std::string tradeMessage(const TradeCommand& command, const TradeResult& result);

public:
    MerchantChat(WorldManager& world, const MerchantCatalog& catalog, const MerchantTrade& trade);

    std::optional<std::vector<ChatMessageData>> interact(const ChatCommandContext& context, Direction direction);
    std::vector<ChatMessageData> execute(const ChatCommandContext& context, const TradeCommand& command);
    void onDisconnect(uint16_t playerId) noexcept;
    void clearConversations() noexcept;
};
