#pragma once

#include "community_config.h"
#include "community_progress.h"

class CommunityCenter {
    CommunityConfig config;
    CommunityProgress progress;
    uint64_t revision = 0;

    void checkBundles();

public:
    explicit CommunityCenter(const CommunityConfig& config = {});

    const CommunityConfig& getConfig() const;
    const CommunityProgress& getProgress() const;
    uint64_t getRevision() const;
    bool isComplete(CommunityBundle bundle) const;
    bool finished() const;
    uint32_t remaining(ItemId item) const;
    uint32_t remainingCoins() const;

    void donate(ItemId item, uint32_t quantity);
    void donateCoins(uint32_t quantity);
    void observeCombat(const CombatProgressData& combat);
    void complete(CommunityBundle bundle);

    void validate(const CommunityProgress& saved) const;
    void restore(const CommunityProgress& saved);
};
