#include "community_state.h"

#include <utility>

#include "community_donations.h"
#include "player/player.h"

CommunityState::CommunityState(const CommunityCenter& center, const CommunityRewards& rewards) :
    center(center), rewards(rewards) {}

CommunityRewardData CommunityState::rewardFor(CommunityBundle bundle) const {
    CommunityRewardData reward;
    switch (bundle) {
        case CommunityBundle::Crops:
            for (const auto seed : community_bundle::cropRewardSeeds) {
                reward.items.push_back({seed, center.getConfig().seedReward, Quality::None});
            }
            break;
        case CommunityBundle::Metals:
            reward.coins = center.getConfig().metalGoldReward;
            break;
        case CommunityBundle::Animals:
            reward.cows = 1;
            reward.chickens = 2;
            break;
        case CommunityBundle::Combat:
            reward.statsBonusPercent = 25;
            break;
        case CommunityBundle::Money:
            reward.marketDiscountNextDay = true;
            break;
    }
    return reward;
}

CommunityStateData CommunityState::forPlayer(
    const Player& player, const std::string& name, bool actionsBlocked) const {
    CommunityStateData state;
    state.available = true;
    state.insideCenter = CommunityDonations::isInsideCenter(player);
    state.completed = center.finished();
    state.canDonate = state.insideCenter && player.canAct() && !actionsBlocked && !state.completed;
    state.discount = rewards.discountState();
    const auto& progress = center.getProgress();
    const auto personal = rewards.pendingFor(name);
    state.coinsDonated = progress.coins;
    state.coinsRequired = center.getConfig().moneyRequired;
    state.combat = progress.combat;
    state.pendingCoins = personal.pendingCoins;
    for (const auto& [seed, amount] : personal.pendingSeeds) {
        if (amount != 0) state.pendingItems.push_back({seed, amount, Quality::None});
    }
    for (const auto bundle : community_bundle::all) {
        CommunityBundleData entry;
        entry.id = bundle;
        entry.completed = center.isComplete(bundle);
        entry.rewardGranted = personal.granted.contains(bundle);
        entry.reward = rewardFor(bundle);
        for (const auto& [item, required] : center.getConfig().requirements) {
            if (community_bundle::forItem(item) == bundle) {
                entry.items.push_back({item, progress.donations.at(item), required,
                    bundle == CommunityBundle::Crops ? Quality::Incredible : Quality::None});
            }
        }
        state.bundles.push_back(std::move(entry));
    }
    return state;
}
