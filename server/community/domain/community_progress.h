#pragma once

#include <cstdint>
#include <map>
#include <set>

#include "community_bundle.h"
#include "dtos.h"

struct CommunityProgress {
    std::map<ItemId, uint32_t> donations;
    uint32_t coins = 0;
    CombatProgressData combat;
    std::set<CommunityBundle> completed;
};
