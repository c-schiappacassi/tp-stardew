#pragma once

#include <map>
#include <string>

#include "community/domain/community_center.h"
#include "commerce/market_prices.h"
#include "community_reward_data.h"

class Player;
class WorldManager;
enum class AnimalSpecies;

class CommunityRewards {
    const CommunityCenter& center;
    WorldManager& world;
    MarketPrices& market;
    std::map<std::string, CommunityPlayerRewards> players;
    bool marketRewardScheduled = false;
    uint64_t revision = 0;

    uint64_t grantAnimals(AnimalSpecies species, uint16_t quantity);
    void unlock(Player& player, CommunityBundle bundle, CommunityPlayerRewards& rewards);

public:
    CommunityRewards(const CommunityCenter& center, WorldManager& world, MarketPrices& market);
    void updateMarket();
    CommunityRewardDelivery collect(Player& player, const std::string& name);
    CommunityPlayerRewards pendingFor(const std::string& name) const;
    const std::map<std::string, CommunityPlayerRewards>& getPlayers() const;
    MarketDiscount discountState() const;
    uint64_t getRevision() const;

    void validate(const std::map<std::string, CommunityPlayerRewards>& saved,const CommunityProgress& progress, MarketDiscount discount) const;
    void restore(std::map<std::string, CommunityPlayerRewards> saved, MarketDiscount discount);
};
