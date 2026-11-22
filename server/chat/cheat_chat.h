#pragma once

#include <functional>

#include "services/cheat_chat_service.h"

class WorldManager;

class CheatChat final : public CheatChatService {
    WorldManager& world;
    std::function<void()> endDay;

    std::string giveItem(Player& player, const CheatCommand& command) const;
    std::string warp(const ChatCommandContext& context, const CheatCommand& command) const;

public:
    explicit CheatChat(WorldManager& world, std::function<void()> endDay = {});
    std::vector<ChatMessageData> execute(
        const ChatCommandContext& context, const CheatCommand& command) override;
};
