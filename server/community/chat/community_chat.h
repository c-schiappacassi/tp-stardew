#pragma once

#include <set>
#include <string>

#include "chat/services/community_chat_service.h"
#include "community/application/community_donations.h"
#include "community/application/community_rewards.h"
#include "community/application/community_state.h"

class CommunityChat final : public CommunityChatService {
    CommunityCenter& center;
    CommunityDonations donations;
    CommunityRewards& rewards;
    CommunityState snapshot;
    std::set<uint16_t> visitors;
    std::set<std::string> victoryRecipients;

    std::vector<ChatMessageData> execute(const ChatCommandContext&, const CommunityStatusCommand&);
    std::vector<ChatMessageData> execute(const ChatCommandContext&, const CommunityRewardsCommand&);
    std::vector<ChatMessageData> execute(const ChatCommandContext&, const DonateItemCommand&);
    std::vector<ChatMessageData> execute(const ChatCommandContext&, const DonateCoinsCommand&);

public:
    CommunityChat(CommunityCenter& center, CommunityRewards& rewards);
    std::vector<ChatMessageData> execute(const ChatCommandContext& context, const CommunityCommand& command) override;
    std::vector<ChatMessageData> completeCheat(const CheatCommand& command) override;
    std::vector<ChatMessageData> locationMessages(const ChatCommandContext& context) override;
    std::vector<ChatMessageData> collectRewards(const ChatCommandContext& context) override;
    CommunityDonationReply donate(const ChatCommandContext& context, const DonateDTO& command) override;
    CommunityStateData state(const ChatCommandContext& context) const override;
    void onDisconnect(uint16_t playerId) noexcept override;
};
