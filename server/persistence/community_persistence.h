#pragma once

#include <cstdint>
#include <set>
#include <string>

#include "community/application/community_reward_data.h"
#include "community_save_data.h"

class CommunityCenter;
class CommunityRewards;

class CommunityPersistence {
private:
    static uint32_t counter(const CommunitySaveData& data, const std::string& section,
                     const std::string& action, const std::string& key);
    static bool flag(const CommunitySaveData& data, const std::string& action, const std::string& key);
    static uint32_t rewardMask(const std::set<CommunityBundle>& granted);
    static CommunityPlayerRewards readRewards(const CommunitySaveData& data, const std::string& name);

public:
    static CommunitySaveData save(const CommunityCenter& center, const CommunityRewards& rewards);
    static void restore(CommunityCenter& center, CommunityRewards& rewards, const CommunitySaveData& data);
};
