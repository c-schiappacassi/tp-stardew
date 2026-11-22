#include "community_persistence.h"

#include <stdexcept>
#include <utility>

#include "commerce/shop_products.h"
#include "community/application/community_rewards.h"

namespace {
uint32_t counter(const CommunitySaveData& data, const std::string& section,
                 const std::string& action, const std::string& key) {
    const auto part = data.find(section);
    if (part == data.end()) return 0;
    const auto values = part->second.find(action);
    if (values == part->second.end()) return 0;
    const auto value = values->second.find(key);
    return value == values->second.end() ? 0 : value->second;
}

bool flag(const CommunitySaveData& data, const std::string& action, const std::string& key) {
    const auto value = counter(data, "progress", action, key);
    if (value > 1) throw std::invalid_argument("Invalid saved community flag");
    return value != 0;
}

uint32_t rewardMask(const std::set<CommunityBundle>& granted) {
    uint32_t mask = 0;
    uint32_t bit = 1;
    for (const auto bundle : community_bundle::all) {
        if (granted.contains(bundle)) mask |= bit;
        bit <<= 1;
    }
    return mask;
}

CommunityPlayerRewards readRewards(const CommunitySaveData& data, const std::string& name) {
    CommunityPlayerRewards rewards;
    const auto mask = counter(data, "rewards", name, "granted");
    constexpr uint32_t validMask = (1u << community_bundle::all.size()) - 1;
    if ((mask & ~validMask) != 0) throw std::invalid_argument("Invalid saved reward mask");
    uint32_t bit = 1;
    for (const auto bundle : community_bundle::all) {
        if ((mask & bit) != 0) rewards.granted.insert(bundle);
        bit <<= 1;
    }
    rewards.pendingCoins = counter(data, "rewards", name, "gold_low") |
        (static_cast<uint64_t>(counter(data, "rewards", name, "gold_high")) << 32);
    for (const auto seed : community_bundle::cropRewardSeeds) {
        const auto quantity = counter(data, "rewards", name, std::string(shop_products::name(seed)));
        if (quantity > UINT16_MAX) throw std::invalid_argument("Invalid saved seed quantity");
        rewards.pendingSeeds.emplace(seed, static_cast<uint16_t>(quantity));
    }
    return rewards;
}
}

CommunitySaveData CommunityPersistence::save(const CommunityCenter& center, const CommunityRewards& rewards) {
    CommunitySaveData data;
    const auto& progress = center.getProgress();
    for (const auto& [item, quantity] : progress.donations) {
        data["progress"]["items"][std::string(shop_products::name(item))] = quantity;
    }
    data["progress"]["money"]["coins"] = progress.coins;
    data["progress"]["combat"] = {{"skeletons", progress.combat.skeletons},
        {"vampires", progress.combat.vampires}, {"orcs", progress.combat.orcs}, {"skull", progress.combat.skull}};
    for (const auto bundle : community_bundle::all) {
        data["progress"]["completed"][std::string(community_bundle::name(bundle))] = center.isComplete(bundle);
    }
    data["progress"]["market"]["discount"] = static_cast<uint32_t>(rewards.discountState());
    for (const auto& [name, pending] : rewards.getPlayers()) {
        auto& saved = data["rewards"][name];
        saved["granted"] = rewardMask(pending.granted);
        saved["gold_low"] = static_cast<uint32_t>(pending.pendingCoins);
        saved["gold_high"] = static_cast<uint32_t>(pending.pendingCoins >> 32);
        for (const auto seed : community_bundle::cropRewardSeeds) {
            const auto quantity = pending.pendingSeeds.find(seed);
            saved[std::string(shop_products::name(seed))] =
                quantity == pending.pendingSeeds.end() ? 0 : quantity->second;
        }
    }
    return data;
}

void CommunityPersistence::restore(CommunityCenter& center, CommunityRewards& rewards, const CommunitySaveData& data) {
    CommunityProgress progress;
    for (const auto& [item, required] : center.getConfig().requirements) {
        progress.donations.emplace(item, counter(data, "progress", "items", std::string(shop_products::name(item))));
    }
    progress.coins = counter(data, "progress", "money", "coins");
    progress.combat = {flag(data, "combat", "skeletons"), flag(data, "combat", "vampires"),
                       flag(data, "combat", "orcs"), flag(data, "combat", "skull")};
    for (const auto bundle : community_bundle::all) {
        if (flag(data, "completed", std::string(community_bundle::name(bundle)))) progress.completed.insert(bundle);
    }
    std::map<std::string, CommunityPlayerRewards> players;
    if (const auto section = data.find("rewards"); section != data.end()) {
        for (const auto& [name, saved] : section->second) players.emplace(name, readRewards(data, name));
    }
    const auto discount = counter(data, "progress", "market", "discount");
    if (discount > static_cast<uint32_t>(MarketDiscount::Active)) {
        throw std::invalid_argument("Invalid saved market discount");
    }
    const auto marketDiscount = static_cast<MarketDiscount>(discount);

    // Validate the complete snapshot before changing either component.
    center.validate(progress);
    rewards.validate(players, progress, marketDiscount);
    center.restore(progress);
    rewards.restore(std::move(players), marketDiscount);
}
