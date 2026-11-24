#include "community_messages.h"

#include "commerce/shop_products.h"

ChatMessageData CommunityMessages::message(const std::string& text) { return {"Community Center", text}; }

std::vector<ChatMessageData> CommunityMessages::status(const CommunityCenter& center, const std::string& name) {
    std::vector<ChatMessageData> result;
    if (name.empty()) {
        result.push_back(message("Community center: shared progress for this game."));
        for (const auto bundle : community_bundle::all) {
            result.push_back(message(std::string(community_bundle::name(bundle)) +
                (center.isComplete(bundle) ? ": COMPLETED" : ": pending")));
        }
        result.push_back(message("Details: /center crops|metals|animals|combat|money"));
        result.push_back(message("Inside the center: /donate product amount | /donate_coins amount"));
        return result;
    }
    const auto bundle = community_bundle::fromName(name);
    if (!bundle) return {message("Unknown bundle. Use crops, metals, animals, combat or money.")};
    result.push_back(message("Bundle " + std::string(community_bundle::name(*bundle)) +
        (center.isComplete(*bundle) ? ": COMPLETED" : ": pending")));
    const auto& config = center.getConfig();
    const auto& progress = center.getProgress();
    for (const auto& [item, required] : config.requirements) {
        if (community_bundle::forItem(item) != bundle) continue;
        result.push_back(message(std::string(shop_products::name(item)) + ": " +
            std::to_string(progress.donations.at(item)) + "/" + std::to_string(required)));
    }
    switch (*bundle) {
        case CommunityBundle::Crops:
            result.push_back(message("Incredible or perfect quality only. Reward: " +
                std::to_string(config.seedReward) + " seeds of each crop per player."));
            break;
        case CommunityBundle::Metals:
            result.push_back(message("Reward: " + std::to_string(config.metalGoldReward) + " coins per player."));
            result.push_back(message("gold is the mineral; donate currency with /donate_coins."));
            break;
        case CommunityBundle::Animals:
            result.push_back(message("Reward per player: 1 cow and 2 chickens, or their purchase price if there is no room."));
            break;
        case CommunityBundle::Combat: {
            const auto condition = [&](bool complete, const std::string& description) {
                result.push_back(message(std::string(complete ? "[OK] " : "[ ] ") + description));
            };
            condition(progress.combat.skeletons, "3 skeletons from consecutive rooms.");
            condition(progress.combat.vampires, "2 vampires from the same pair.");
            condition(progress.combat.orcs, "4 orcs from the same group.");
            condition(progress.combat.skull, "Survive a room with a skull without taking damage.");
            result.push_back(message("Tracked automatically. Reward: +25% maximum health and energy."));
            break;
        }
        case CommunityBundle::Money:
            result.push_back(message("Coins: " + std::to_string(progress.coins) + "/" + std::to_string(config.moneyRequired)));
            result.push_back(message("Reward the next day: buy at half price and sell for double for one day."));
            break;
    }
    return result;
}

ChatMessageData CommunityMessages::donation(const CommunityCenter& center,
    const CommunityDonationResult& result, std::optional<ItemId> item) {
    switch (result.error) {
        case CommunityDonationError::InvalidAmount: return message("The amount must be greater than zero.");
        case CommunityDonationError::CannotAct: return message("You cannot donate right now.");
        case CommunityDonationError::OutsideCenter:
            return message("Enter the community center to donate. Check /center from anywhere.");
        case CommunityDonationError::UnsupportedItem:
            return message("That product is not a community center requirement. Check /center.");
        case CommunityDonationError::AlreadyComplete:
            return message("That requirement is already complete. Nothing was consumed.");
        case CommunityDonationError::MissingItems:
            return message("You do not have enough items of the required quality. No items were consumed.");
        case CommunityDonationError::MissingCoins:
            return message("You do not have enough coins. No coins were deducted.");
        case CommunityDonationError::InventoryChanged: return message("The donation could not be completed.");
        case CommunityDonationError::InvalidSlot: return message("Choose an inventory slot containing the item to donate.");
        case CommunityDonationError::None: break;
    }
    const auto& config = center.getConfig();
    const auto& progress = center.getProgress();
    const std::string product = item ? std::string(shop_products::name(*item)) : "coins";
    const auto donated = item ? progress.donations.at(*item) : progress.coins;
    const auto required = item ? config.requirements.at(*item) : config.moneyRequired;
    return message("You donated " + std::to_string(result.amount) + " " + product + ". Progress: " +
        std::to_string(donated) + "/" + std::to_string(required) + ".");
}

std::vector<ChatMessageData> CommunityMessages::delivery(const CommunityRewardDelivery& received) {
    std::vector<ChatMessageData> result;
    for (const auto bundle : received.unlocked) {
        result.push_back(message("Bundle " + std::string(community_bundle::name(bundle)) +
            " completed. Your reward is unlocked."));
        if (bundle == CommunityBundle::Crops) {
            result.push_back(message("Seeds that do not fit remain pending. Check /rewards."));
        }
    }
    if (received.coins != 0) result.push_back(message("You received " + std::to_string(received.coins) + " reward coins."));
    for (const auto& [seed, quantity] : received.seeds) {
        result.push_back(message("You received " + std::to_string(quantity) + " " +
            std::string(shop_products::name(seed)) + "."));
    }
    return result;
}

std::vector<ChatMessageData> CommunityMessages::pending(const CommunityPlayerRewards& rewards) {
    std::vector<ChatMessageData> result;
    if (rewards.pendingCoins != 0) result.push_back(message("Pending coins: " +
        std::to_string(rewards.pendingCoins) + ". Make room in your wallet."));
    for (const auto& [seed, quantity] : rewards.pendingSeeds) {
        if (quantity != 0) result.push_back(message(std::string(shop_products::name(seed)) +
            " pending: " + std::to_string(quantity) + ". Make room in your inventory."));
    }
    return result;
}
