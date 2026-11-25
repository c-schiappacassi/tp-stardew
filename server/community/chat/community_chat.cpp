#include "community_chat.h"

#include "server_constants.h"

#include "commerce/shop_products.h"
#include "player/player_manager.h"

#include "community_messages.h"

CommunityChat::CommunityChat(CommunityCenter& center, CommunityRewards& rewards): center(center), donations(center), rewards(rewards), snapshot(center, rewards) {}

std::vector<ChatMessageData> CommunityChat::execute(const ChatCommandContext& context, const CommunityCommand& command) {
    return std::visit([&](const auto& request) { return execute(context, request); }, command);
}

std::vector<ChatMessageData> CommunityChat::execute(const ChatCommandContext&, const CommunityStatusCommand& command) { return CommunityMessages::status(center, command.bundle); }

std::vector<ChatMessageData> CommunityChat::execute(const ChatCommandContext& context, const CommunityRewardsCommand&) {
    auto messages = collectRewards(context);
    const auto pending = CommunityMessages::pending(rewards.pendingFor(context.playerName));
    messages.insert(messages.end(), pending.begin(), pending.end());
    if (messages.empty()) {
        messages.push_back(CommunityMessages::message(server_constants::ALL_YOUR_UNLOCKED_REWARDS_HAVE_BEEN_DELIVERED));
    }
    return messages;
}

std::vector<ChatMessageData> CommunityChat::execute(const ChatCommandContext& context, const DonateCoinsCommand& command) {
    const auto result = donations.donateCoins(context.players, context.playerId, command.quantity, context.actionsBlocked);
    return {CommunityMessages::donation(center, result)};
}

std::vector<ChatMessageData> CommunityChat::execute(const ChatCommandContext& context, const DonateItemCommand& command) {
    const auto item = shop_products::item(command.product);
    if (!item) {
        return {CommunityMessages::donation(center, {CommunityDonationError::UnsupportedItem, 0})};
    }
    const auto result = donations.donateItem(context.players, context.playerId, *item, command.quantity, context.actionsBlocked);
    return {CommunityMessages::donation(center, result, item)};
}

std::vector<ChatMessageData> CommunityChat::completeCheat(const CheatCommand& command) {
    if (command.action == CheatAction::CompleteAllBundles) {
        for (const auto bundle: community_bundle::all) center.complete(bundle);
    } else if (command.action == CheatAction::CompleteBundle) {
        const auto bundle = community_bundle::fromName(command.target);
        if (!bundle)
            return {CommunityMessages::message(server_constants::UNKNOWN_BUNDLE_USE_CROPS_METALS_ANIMALS_COMBAT_OR_MONEY)};
        center.complete(*bundle);
    } else {
        return {CommunityMessages::message(server_constants::UNKNOWN_COMMUNITY_CENTER_CHEAT)};
    }
    return {CommunityMessages::message(server_constants::BUNDLES_UPDATED_REWARDS_ARE_GRANTED_ONLY_ONCE_PER_PLAYER)};
}

std::vector<ChatMessageData> CommunityChat::locationMessages(const ChatCommandContext& context) {
    if (!CommunityDonations::isInsideCenter(context.players.snapshot(context.playerId))) {
        visitors.erase(context.playerId);
        return {};
    }
    if (!visitors.insert(context.playerId).second)
        return {};
    return {CommunityMessages::message(server_constants::WELCOME_CENTER_LISTS_THE_BUNDLES_CENTER_METALS_SHOWS_THE_DETAILS),
        CommunityMessages::message(server_constants::COMMAND_DONATE_PRODUCT_AMOUNT_DONATE_COINS_AMOUNT_REWARDS)};
}

std::vector<ChatMessageData> CommunityChat::collectRewards(const ChatCommandContext& context) {
    auto messages = CommunityMessages::delivery(rewards.collect(context.players, context.playerId, context.playerName));
    if (center.finished() && victoryRecipients.insert(context.playerName).second) {
        messages.push_back(CommunityMessages::message(server_constants::CONGRATULATIONS_ALL_COMMUNITY_CENTER_BUNDLES_ARE_COMPLETE_YOU_FINISHED_THE_GAME));
    }
    return messages;
}

CommunityDonationReply CommunityChat::donate(const ChatCommandContext& context, const DonateDTO& command) {
    CommunityDonationResult result;
    std::optional<ItemId> item;
    if (command.kind == DonationKind::Coins) {
        result = donations.donateCoins(context.players, context.playerId, command.quantity, context.actionsBlocked);
    } else if (command.kind == DonationKind::Item) {
        const auto items = context.players.inventory(context.playerId);
        const auto selected = items.find(command.slot);
        if (selected != items.end())
            item = selected->second.item_id;
        result = donations.donateSlot(context.players, context.playerId, command.slot, command.quantity, context.actionsBlocked);
    } else {
        result.error = CommunityDonationError::UnsupportedItem;
    }
    return {{command.requestId, command.kind, command.slot, result.error, result.amount}, {CommunityMessages::donation(center, result, item)}};
}

CommunityStateData CommunityChat::state(const ChatCommandContext& context) const {
    return snapshot.forPlayer(context.players.snapshot(context.playerId), context.playerName, context.actionsBlocked);
}

void CommunityChat::onDisconnect(uint16_t playerId) noexcept { visitors.erase(playerId); }
