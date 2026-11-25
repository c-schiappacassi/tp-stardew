#include "community_messages.h"

#include "server_constants.h"

#include "commerce/shop_products.h"

ChatMessageData CommunityMessages::message(const std::string& text) { return {server_constants::COMMUNITY_CENTER, text}; }

std::vector<ChatMessageData> CommunityMessages::status(const CommunityCenter& center, const std::string& name) {
    std::vector<ChatMessageData> result;
    if (name.empty()) {
        result.push_back(message(server_constants::COMMUNITY_CENTER_SHARED_PROGRESS_FOR_THIS_GAME));
        for (const auto bundle : community_bundle::all) {
            result.push_back(message(std::string(community_bundle::name(bundle)) +
                (center.isComplete(bundle) ? server_constants::COMPLETED : server_constants::PENDING)));
        }
        result.push_back(message(server_constants::DETAILS_CENTER_CROPS_METALS_ANIMALS_COMBAT_MONEY));
        result.push_back(message(server_constants::INSIDE_THE_CENTER_DONATE_PRODUCT_AMOUNT_DONATE_COINS_AMOUNT));
        return result;
    }
    const auto bundle = community_bundle::fromName(name);
    if (!bundle) return {message(server_constants::UNKNOWN_BUNDLE_USE_CROPS_METALS_ANIMALS_COMBAT_OR_MONEY)};
    result.push_back(message(server_constants::BUNDLE_PREFIX + std::string(community_bundle::name(*bundle)) +
        (center.isComplete(*bundle) ? server_constants::COMPLETED : server_constants::PENDING)));
    const auto& config = center.getConfig();
    const auto& progress = center.getProgress();
    for (const auto& [item, required] : config.requirements) {
        if (community_bundle::forItem(item) != bundle) continue;
        result.push_back(message(std::string(shop_products::name(item)) + server_constants::COLON_SPACE +
            std::to_string(progress.donations.at(item)) + server_constants::SLASH + std::to_string(required)));
    }
    switch (*bundle) {
        case CommunityBundle::Crops:
            result.push_back(message(server_constants::INCREDIBLE_OR_PERFECT_QUALITY_ONLY_REWARD_PREFIX +
                std::to_string(config.seedReward) + server_constants::SEEDS_OF_EACH_CROP_PER_PLAYER_SUFFIX));
            break;
        case CommunityBundle::Metals:
            result.push_back(message(server_constants::REWARD_PREFIX + std::to_string(config.metalGoldReward) + server_constants::COINS_PER_PLAYER_SUFFIX));
            result.push_back(message(server_constants::GOLD_IS_THE_MINERAL_DONATE_CURRENCY_WITH_DONATE_COINS));
            break;
        case CommunityBundle::Animals:
            result.push_back(message(server_constants::REWARD_PER_PLAYER_1_COW_AND_2_CHICKENS_OR_THEIR_PURCHASE_PRICE));
            break;
        case CommunityBundle::Combat: {
            const auto condition = [&](bool complete, const std::string& description) {
                result.push_back(message(std::string(complete ? server_constants::OK_PREFIX : server_constants::UNCHECKED_BOX_PREFIX) + description));
            };
            condition(progress.combat.skeletons, server_constants::TEXT_3_SKELETONS_FROM_CONSECUTIVE_ROOMS);
            condition(progress.combat.vampires, server_constants::TEXT_2_VAMPIRES_FROM_THE_SAME_PAIR);
            condition(progress.combat.orcs, server_constants::TEXT_4_ORCS_FROM_THE_SAME_GROUP);
            condition(progress.combat.skull, server_constants::SURVIVE_A_ROOM_WITH_A_SKULL_WITHOUT_TAKING_DAMAGE);
            result.push_back(message(server_constants::TRACKED_AUTOMATICALLY_REWARD_25_MAXIMUM_HEALTH_AND_ENERGY));
            break;
        }
        case CommunityBundle::Money:
            result.push_back(message(server_constants::COINS_PREFIX + std::to_string(progress.coins) + server_constants::SLASH + std::to_string(config.moneyRequired)));
            result.push_back(message(server_constants::REWARD_THE_NEXT_DAY_BUY_AT_HALF_PRICE_AND_SELL_FOR_DOUBLE));
            break;
    }
    return result;
}

ChatMessageData CommunityMessages::donation(const CommunityCenter& center,
    const CommunityDonationResult& result, std::optional<ItemId> item) {
    switch (result.error) {
        case CommunityDonationError::InvalidAmount: return message(server_constants::THE_AMOUNT_MUST_BE_GREATER_THAN_ZERO);
        case CommunityDonationError::CannotAct: return message(server_constants::YOU_CANNOT_DONATE_RIGHT_NOW);
        case CommunityDonationError::OutsideCenter:
            return message(server_constants::ENTER_THE_COMMUNITY_CENTER_TO_DONATE_CHECK_CENTER_FROM_ANYWHERE);
        case CommunityDonationError::UnsupportedItem:
            return message(server_constants::THAT_PRODUCT_IS_NOT_A_COMMUNITY_CENTER_REQUIREMENT_CHECK_CENTER);
        case CommunityDonationError::AlreadyComplete:
            return message(server_constants::THAT_REQUIREMENT_IS_ALREADY_COMPLETE_NOTHING_WAS_CONSUMED);
        case CommunityDonationError::MissingItems:
            return message(server_constants::YOU_DO_NOT_HAVE_ENOUGH_ITEMS_OF_THE_REQUIRED_QUALITY_NO_ITEMS);
        case CommunityDonationError::MissingCoins:
            return message(server_constants::YOU_DO_NOT_HAVE_ENOUGH_COINS_NO_COINS_WERE_DEDUCTED);
        case CommunityDonationError::InventoryChanged: return message(server_constants::THE_DONATION_COULD_NOT_BE_COMPLETED);
        case CommunityDonationError::InvalidSlot: return message(server_constants::CHOOSE_AN_INVENTORY_SLOT_CONTAINING_THE_ITEM_TO_DONATE);
        case CommunityDonationError::None: break;
    }
    const auto& config = center.getConfig();
    const auto& progress = center.getProgress();
    const std::string product = item ? std::string(shop_products::name(*item)) : server_constants::COINS;
    const auto donated = item ? progress.donations.at(*item) : progress.coins;
    const auto required = item ? config.requirements.at(*item) : config.moneyRequired;
    return message(server_constants::YOU_DONATED_PREFIX + std::to_string(result.amount) + server_constants::SPACE + product + server_constants::FIELD_PROGRESS_PREFIX +
        std::to_string(donated) + server_constants::SLASH + std::to_string(required) + server_constants::DOT);
}

std::vector<ChatMessageData> CommunityMessages::delivery(const CommunityRewardDelivery& received) {
    std::vector<ChatMessageData> result;
    for (const auto bundle : received.unlocked) {
        result.push_back(message(server_constants::BUNDLE_PREFIX + std::string(community_bundle::name(bundle)) +
            server_constants::COMPLETED_YOUR_REWARD_IS_UNLOCKED_SUFFIX));
        if (bundle == CommunityBundle::Crops) {
            result.push_back(message(server_constants::SEEDS_THAT_DO_NOT_FIT_REMAIN_PENDING_CHECK_REWARDS));
        }
    }
    if (received.coins != 0) result.push_back(message(server_constants::YOU_RECEIVED_PREFIX + std::to_string(received.coins) + server_constants::REWARD_COINS_SUFFIX));
    for (const auto& [seed, quantity] : received.seeds) {
        result.push_back(message(server_constants::YOU_RECEIVED_PREFIX + std::to_string(quantity) + server_constants::SPACE +
            std::string(shop_products::name(seed)) + server_constants::DOT));
    }
    return result;
}

std::vector<ChatMessageData> CommunityMessages::pending(const CommunityPlayerRewards& rewards) {
    std::vector<ChatMessageData> result;
    if (rewards.pendingCoins != 0) result.push_back(message(server_constants::PENDING_COINS_PREFIX +
        std::to_string(rewards.pendingCoins) + server_constants::FIELD_MAKE_ROOM_IN_YOUR_WALLET));
    for (const auto& [seed, quantity] : rewards.pendingSeeds) {
        if (quantity != 0) result.push_back(message(std::string(shop_products::name(seed)) +
            server_constants::PENDING_SEPARATOR + std::to_string(quantity) + server_constants::FIELD_MAKE_ROOM_IN_YOUR_INVENTORY));
    }
    return result;
}
