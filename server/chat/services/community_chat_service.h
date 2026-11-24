#pragma once

#include <vector>

#include "chat/chat_command_context.h"
#include "chat/commands/cheat_command.h"
#include "chat/commands/community_command.h"
#include "dtos.h"

struct CommunityDonationReply {
    CommunityDonationReceipt receipt;
    std::vector<ChatMessageData> messages;
};

class CommunityChatService {
public:
    virtual ~CommunityChatService() = default;

    virtual std::vector<ChatMessageData> execute(const ChatCommandContext& context, const CommunityCommand& command) = 0;
    virtual std::vector<ChatMessageData> completeCheat(const CheatCommand& command) = 0;
    virtual std::vector<ChatMessageData> locationMessages(const ChatCommandContext& context) = 0;
    virtual std::vector<ChatMessageData> collectRewards(const ChatCommandContext& context) = 0;
    virtual CommunityDonationReply donate(const ChatCommandContext& context, const DonateDTO& command) = 0;
    virtual CommunityStateData state(const ChatCommandContext& context) const = 0;
    virtual void onDisconnect(uint16_t) noexcept {}
};
