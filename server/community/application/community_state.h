#pragma once

#include <string>

#include "community_rewards.h"
#include "dtos.h"

class PlayerSnapshot;

class CommunityState {
    const CommunityCenter& center;
    const CommunityRewards& rewards;

    CommunityRewardData rewardFor(CommunityBundle bundle) const;

public:
    CommunityState(const CommunityCenter& center, const CommunityRewards& rewards);
    CommunityStateData forPlayer(const PlayerSnapshot& player, const std::string& name, bool actionsBlocked) const;
};
