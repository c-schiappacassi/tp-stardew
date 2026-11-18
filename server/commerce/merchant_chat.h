#pragma once

#include <map>

#include "chat/chat_commands.h"
#include "shop_config.h"

class WorldManager;

class MerchantChat final : public MerchantChatService {
    struct Conversation {
        InteriorLocation location;
        NpcData npc;
    };

    WorldManager& world;
    MarketConfig config;
    std::map<uint16_t, Conversation> conversations;
    std::vector<RobinProductConfig> robinCatalog;
    std::size_t rotation = 0;

    void rotateCatalog();
    std::vector<ChatMessageData> catalog(NpcId merchant) const;
    std::string buy(Player& player, NpcId merchant, const TradeCommand& command);
    std::string sell(Player& player, NpcId merchant, const TradeCommand& command);

public:
    explicit MerchantChat(WorldManager& world, const MarketConfig& config = {});
    std::optional<std::vector<ChatMessageData>> interact(const ChatCommandContext& context, Direction direction) override;
    std::vector<ChatMessageData> execute(const ChatCommandContext& context, const TradeCommand& command) override;
    void onDisconnect(uint16_t playerId) noexcept override;
    void finishDay();
};
