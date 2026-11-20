#pragma once

#include "community_save_data.h"

class CommunityCenter;
class CommunityRewards;

class CommunityPersistence {
public:
    static CommunitySaveData save(const CommunityCenter& center, const CommunityRewards& rewards);
    static void restore(CommunityCenter& center, CommunityRewards& rewards, const CommunitySaveData& data);
};
