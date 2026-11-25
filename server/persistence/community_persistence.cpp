#include "community_persistence.h"

#include "server_constants.h"

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
    const auto value = counter(data, server_constants::PROGRESS, action, key);
    if (value > 1) throw std::invalid_argument(server_constants::INVALID_SAVED_COMMUNITY_FLAG);
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
    const auto mask = counter(data, server_constants::REWARDS, name, server_constants::GRANTED);
    constexpr uint32_t validMask = (1u << community_bundle::all.size()) - 1;
    if ((mask & ~validMask) != 0) throw std::invalid_argument(server_constants::INVALID_SAVED_REWARD_MASK);
    uint32_t bit = 1;
    for (const auto bundle : community_bundle::all) {
        if ((mask & bit) != 0) rewards.granted.insert(bundle);
        bit <<= 1;
    }
    rewards.pendingCoins = counter(data, server_constants::REWARDS, name, server_constants::GOLD_LOW) |
        (static_cast<uint64_t>(counter(data, server_constants::REWARDS, name, server_constants::GOLD_HIGH)) << 32);
    for (const auto seed : community_bundle::cropRewardSeeds) {
        const auto quantity = counter(data, server_constants::REWARDS, name, std::string(shop_products::name(seed)));
        if (quantity > UINT16_MAX) throw std::invalid_argument(server_constants::INVALID_SAVED_SEED_QUANTITY);
        rewards.pendingSeeds.emplace(seed, static_cast<uint16_t>(quantity));
    }
    return rewards;
}
}

CommunitySaveData CommunityPersistence::save(const CommunityCenter& center, const CommunityRewards& rewards) {
    CommunitySaveData data;
    const auto& progress = center.getProgress();
    for (const auto& [item, quantity] : progress.donations) {
        data[server_constants::PROGRESS][server_constants::ITEMS][std::string(shop_products::name(item))] = quantity;
    }
    data[server_constants::PROGRESS][server_constants::MONEY][server_constants::COINS] = progress.coins;
    data[server_constants::PROGRESS][server_constants::COMBAT] = {{server_constants::SKELETONS, progress.combat.skeletons},
        {server_constants::VAMPIRES, progress.combat.vampires}, {server_constants::ORCS, progress.combat.orcs}, {server_constants::SKULL, progress.combat.skull}};
    for (const auto bundle : community_bundle::all) {
        data[server_constants::PROGRESS][server_constants::COMPLETED_2][std::string(community_bundle::name(bundle))] = center.isComplete(bundle);
    }
    data[server_constants::PROGRESS][server_constants::MARKET][server_constants::DISCOUNT] = static_cast<uint32_t>(rewards.discountState());
    for (const auto& [name, pending] : rewards.getPlayers()) {
        auto& saved = data[server_constants::REWARDS][name];
        saved[server_constants::GRANTED] = rewardMask(pending.granted);
        saved[server_constants::GOLD_LOW] = static_cast<uint32_t>(pending.pendingCoins);
        saved[server_constants::GOLD_HIGH] = static_cast<uint32_t>(pending.pendingCoins >> 32);
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
        progress.donations.emplace(item, counter(data, server_constants::PROGRESS, server_constants::ITEMS, std::string(shop_products::name(item))));
    }
    progress.coins = counter(data, server_constants::PROGRESS, server_constants::MONEY, server_constants::COINS);
    progress.combat = {flag(data, server_constants::COMBAT, server_constants::SKELETONS), flag(data, server_constants::COMBAT, server_constants::VAMPIRES),
                       flag(data, server_constants::COMBAT, server_constants::ORCS), flag(data, server_constants::COMBAT, server_constants::SKULL)};
    for (const auto bundle : community_bundle::all) {
        if (flag(data, server_constants::COMPLETED_2, std::string(community_bundle::name(bundle)))) progress.completed.insert(bundle);
    }
    std::map<std::string, CommunityPlayerRewards> players;
    if (const auto section = data.find(server_constants::REWARDS); section != data.end()) {
        for (const auto& [name, saved] : section->second) players.emplace(name, readRewards(data, name));
    }
    const auto discount = counter(data, server_constants::PROGRESS, server_constants::MARKET, server_constants::DISCOUNT);
    if (discount > static_cast<uint32_t>(MarketDiscount::Active)) {
        throw std::invalid_argument(server_constants::INVALID_SAVED_MARKET_DISCOUNT);
    }
    const auto marketDiscount = static_cast<MarketDiscount>(discount);

    // Validate the complete snapshot before changing either component.
    center.validate(progress);
    rewards.validate(players, progress, marketDiscount);
    center.restore(progress);
    rewards.restore(std::move(players), marketDiscount);
}
