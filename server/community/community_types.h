#pragma once

#include <cstdint>
#include <map>
#include <set>

#include "community/domain/community_bundle.h"
#include "dtos.h"
#include "protocol_types.h"

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

struct CommunityDonationResult {
    CommunityDonationError error = CommunityDonationError::None;
    uint32_t amount = 0;
};

struct CommunityProgress {
    std::map<ItemId, uint32_t> donations;
    uint32_t coins = 0;
    CombatProgressData combat;
    std::set<CommunityBundle> completed;
};

struct CommunityConfig {
    std::map<ItemId, uint32_t> requirements{
        {ItemId::Strawberry, 3}, {ItemId::Scallion, 3},
        {ItemId::Potato, 3}, {ItemId::Pumpkin, 3},
        {ItemId::Coal, 10}, {ItemId::Lapis, 7}, {ItemId::Silver, 5}, {ItemId::Gold, 3},
        {ItemId::Milk, 5}, {ItemId::Egg, 10}
    };
    uint32_t moneyRequired = 10000;
    uint16_t seedReward = 5;
    uint32_t metalGoldReward = 500;

    void validate() const;
};
