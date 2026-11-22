#pragma once

#include <optional>
#include <string>
#include <vector>

#include "community/application/community_donation_result.h"
#include "community/application/community_reward_data.h"
#include "community/domain/community_center.h"

class CommunityMessages {
public:
    static ChatMessageData message(const std::string& text);
    static std::vector<ChatMessageData> status(const CommunityCenter& center, const std::string& name);
    static ChatMessageData donation(const CommunityCenter& center, const CommunityDonationResult& result,std::optional<ItemId> item = std::nullopt);
    static std::vector<ChatMessageData> delivery(const CommunityRewardDelivery& received);
    static std::vector<ChatMessageData> pending(const CommunityPlayerRewards& rewards);
};
