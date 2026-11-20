#pragma once

#include <string>

#include "community_rewards.h"
#include "dtos.h"

class Player;

class CommunityState {
    const CommunityCenter& center;
    const CommunityRewards& rewards;

    CommunityRewardData rewardFor(CommunityBundle bundle) const;

public:
    CommunityState(const CommunityCenter& center, const CommunityRewards& rewards);
    CommunityStateData forPlayer(const Player& player, const std::string& name, bool actionsBlocked) const;
};
