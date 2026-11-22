#include "community_rewards.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

#include "player/player_manager.h"
#include "world/world_manager.h"

CommunityRewards::CommunityRewards(const CommunityCenter& center, WorldManager& world, MarketPrices& market): center(center), world(world), market(market) {}

void CommunityRewards::updateMarket() {
    if (center.isComplete(CommunityBundle::Money) && !marketRewardScheduled) {
        market.scheduleDiscount();
        marketRewardScheduled = true;
        ++revision;
    }
}

uint64_t CommunityRewards::grantAnimals(AnimalSpecies species, uint16_t quantity) {
    const auto& definitions = world.getFarm().getConfig().animals;
    const auto definition = std::find_if(definitions.begin(), definitions.end(), [&](const auto& animal) { return animal.species == species; });
    if (definition == definitions.end())
        return 0;
    uint64_t coins = 0;
    for (uint16_t i = 0; i < quantity; ++i) {
        if (!world.buyAnimals(species, 1))
            coins += definition->basePrice;
    }
    return coins;
}

void CommunityRewards::unlock(PlayerManager& playerManager, uint16_t playerId, CommunityBundle bundle, CommunityPlayerRewards& rewards) {
    switch (bundle) {
        case CommunityBundle::Crops:
            for (const auto seed: community_bundle::cropRewardSeeds) {
                rewards.pendingSeeds[seed] = center.getConfig().seedReward;
            }
            break;
        case CommunityBundle::Metals:
            rewards.pendingCoins += center.getConfig().metalGoldReward;
            break;
        case CommunityBundle::Animals:
            rewards.pendingCoins += grantAnimals(AnimalSpecies::Cow, 1);
            rewards.pendingCoins += grantAnimals(AnimalSpecies::Chicken, 2);
            break;
        case CommunityBundle::Combat:
            playerManager.apply(playerId, player_action::ApplyCombatBonus{});
            break;
        case CommunityBundle::Money:
            break;
    }
    rewards.granted.insert(bundle);
}

CommunityRewardDelivery CommunityRewards::collect(PlayerManager& playerManager, uint16_t playerId, const std::string& name) {
    auto& rewards = players[name];
    CommunityRewardDelivery delivery;
    for (const auto bundle: community_bundle::all) {
        if (!center.isComplete(bundle) || rewards.granted.contains(bundle))
            continue;
        unlock(playerManager, playerId, bundle, rewards);
        delivery.unlocked.insert(bundle);
    }
    const auto coins = static_cast<uint32_t>(std::min<uint64_t>(rewards.pendingCoins, std::numeric_limits<uint32_t>::max()));
    delivery.coins = playerManager.apply(playerId, player_action::AddGold{coins}).amount;
    rewards.pendingCoins -= delivery.coins;
    for (auto& [seed, pending]: rewards.pendingSeeds) {
        if (pending == 0)
            continue;
        const auto received = playerManager.apply(playerId, player_action::AddItemUpTo{{seed, pending, Quality::None}}).amount;
        pending -= received;
        if (received != 0)
            delivery.seeds.emplace(seed, received);
    }
    if (!delivery.unlocked.empty() || !delivery.seeds.empty() || delivery.coins != 0)
        ++revision;
    return delivery;
}

CommunityPlayerRewards CommunityRewards::pendingFor(const std::string& name) const {
    const auto found = players.find(name);
    return found == players.end() ? CommunityPlayerRewards{} : found->second;
}

const std::map<std::string, CommunityPlayerRewards>& CommunityRewards::getPlayers() const { return players; }
MarketDiscount CommunityRewards::discountState() const { return market.discountState(); }
uint64_t CommunityRewards::getRevision() const { return revision; }

void CommunityRewards::validate(const std::map<std::string, CommunityPlayerRewards>& saved, const CommunityProgress& progress, MarketDiscount discount) const {
    if (discount > MarketDiscount::Active || (discount != MarketDiscount::None && !progress.completed.contains(CommunityBundle::Money))) {
        throw std::invalid_argument("Invalid saved market discount");
    }
    for (const auto& [name, rewards]: saved) {
        for (const auto bundle: rewards.granted) {
            if (!progress.completed.contains(bundle))
                throw std::invalid_argument("Invalid granted reward");
        }
        for (const auto& [seed, pending]: rewards.pendingSeeds) {
            const auto& seeds = community_bundle::cropRewardSeeds;
            if (std::find(seeds.begin(), seeds.end(), seed) == seeds.end() || pending > center.getConfig().seedReward ||
                (pending != 0 && !rewards.granted.contains(CommunityBundle::Crops))) {
                throw std::invalid_argument("Invalid pending seeds");
            }
        }
        uint64_t maximumCoins = rewards.granted.contains(CommunityBundle::Metals) ? center.getConfig().metalGoldReward : 0;
        if (rewards.granted.contains(CommunityBundle::Animals)) {
            for (const auto& animal: world.getFarm().getConfig().animals) {
                maximumCoins += static_cast<uint64_t>(animal.basePrice) * (animal.species == AnimalSpecies::Cow ? 1 : 2);
            }
        }
        if (rewards.pendingCoins > maximumCoins)
            throw std::invalid_argument("Invalid pending coins");
    }
}

void CommunityRewards::restore(std::map<std::string, CommunityPlayerRewards> saved, MarketDiscount discount) {
    validate(saved, center.getProgress(), discount);
    players = std::move(saved);
    marketRewardScheduled = center.isComplete(CommunityBundle::Money);
    market.restoreDiscount(discount);
    ++revision;
}
