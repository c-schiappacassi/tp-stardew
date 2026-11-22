#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "chat/chat_command_context.h"
#include "chat/commands/trade_command.h"
#include "dtos.h"

class MerchantChatService {
public:
    virtual ~MerchantChatService() = default;

    virtual std::optional<std::vector<ChatMessageData>> interact(
        const ChatCommandContext&, Direction) {
        return std::nullopt;
    }

    virtual std::vector<ChatMessageData> execute(
        const ChatCommandContext& context, const TradeCommand& command) = 0;

    virtual void onDisconnect(uint16_t) noexcept {}
};
