#pragma once

#include <cstdint>
#include <vector>

#include "chat/chat_command_context.h"
#include "chat/commands/cheat_command.h"
#include "dtos.h"

class CheatChatService {
public:
    virtual ~CheatChatService() = default;

    virtual std::vector<ChatMessageData> execute(
        const ChatCommandContext& context, const CheatCommand& command) = 0;

    virtual void onDisconnect(uint16_t) noexcept {}
};
