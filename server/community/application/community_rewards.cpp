#include "community_rewards.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

#include "commerce/commerce_manager.h"
#include "player/player_manager.h"
#include "world/world_manager.h"

CommunityRewards::CommunityRewards(const CommunityCenter& center, WorldManager& world, CommerceManager& commerce): center(center), world(world), commerce(commerce) {}

void CommunityRewards::updateMarket() {
    if (center.isComplete(CommunityBundle::Money) && !marketRewardScheduled) {
        commerce.scheduleDiscount();
        marketRewardScheduled = true;
        ++revision;
    }
}

uint64_t CommunityRewards::grantAnimals(AnimalSpecies species, uint16_t quantity) {
    const auto price = world.getFarm().animalBasePrice(species);
    if (!price)
        return 0;
    uint64_t coins = 0;
    for (uint16_t i = 0; i < quantity; ++i) {
        if (!world.buyAnimals(species, 1))
            coins += *price;
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
MarketDiscount CommunityRewards::discountState() const { return commerce.discountState(); }
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
            maximumCoins += world.getFarm().animalBasePrice(AnimalSpecies::Cow).value_or(0);
            maximumCoins += 2 * static_cast<uint64_t>(world.getFarm().animalBasePrice(AnimalSpecies::Chicken).value_or(0));
        }
        if (rewards.pendingCoins > maximumCoins)
            throw std::invalid_argument("Invalid pending coins");
    }
}

void CommunityRewards::restore(std::map<std::string, CommunityPlayerRewards> saved, MarketDiscount discount) {
    validate(saved, center.getProgress(), discount);
    players = std::move(saved);
    marketRewardScheduled = center.isComplete(CommunityBundle::Money);
    commerce.restoreDiscount(discount);
    ++revision;
}
