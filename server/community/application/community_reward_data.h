#pragma once

#include <cstdint>
#include <map>
#include <set>

#include "community/domain/community_bundle.h"

struct CommunityPlayerRewards {
    std::set<CommunityBundle> granted;
    std::map<ItemId, uint16_t> pendingSeeds;
    uint64_t pendingCoins = 0;
};

struct CommunityRewardDelivery {
    std::set<CommunityBundle> unlocked;
    std::map<ItemId, uint16_t> seeds;
    uint32_t coins = 0;
};
