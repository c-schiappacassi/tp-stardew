#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "services/cheat_chat_service.h"

class WorldManager;

class CheatChat final: public CheatChatService {
    WorldManager& world;
    std::function<void()> endDay;

    static const std::pair<std::string_view, ItemId> tools[4];

    static std::optional<ItemId> itemNamed(const std::string& name);
    static std::vector<ChatMessageData> listGiveItems();

    std::string giveItem(PlayerManager& players, uint16_t playerId, const CheatCommand& command) const;
    std::string warp(const ChatCommandContext& context, const CheatCommand& command) const;

public:
    explicit CheatChat(WorldManager& world, std::function<void()> endDay = {});
    std::vector<ChatMessageData> execute(const ChatCommandContext& context, const CheatCommand& command) override;
};
