#include "community_chat.h"

#include "commerce/shop_products.h"
#include "player/player.h"
#include "community_messages.h"

CommunityChat::CommunityChat(CommunityCenter& center, CommunityRewards& rewards) :
    center(center), donations(center), rewards(rewards), snapshot(center, rewards) {}

std::vector<ChatMessageData> CommunityChat::execute(
    const ChatCommandContext& context, const CommunityCommand& command) {
    return std::visit([&](const auto& request) { return execute(context, request); }, command);
}

std::vector<ChatMessageData> CommunityChat::execute(
    const ChatCommandContext&, const CommunityStatusCommand& command) {
    return CommunityMessages::status(center, command.bundle);
}

std::vector<ChatMessageData> CommunityChat::execute(
    const ChatCommandContext& context, const CommunityRewardsCommand&) {
    auto messages = collectRewards(context);
    const auto pending = CommunityMessages::pending(rewards.pendingFor(context.playerName));
    messages.insert(messages.end(), pending.begin(), pending.end());
    if (messages.empty()) {
        messages.push_back(CommunityMessages::message("All your unlocked rewards have been delivered."));
    }
    return messages;
}

std::vector<ChatMessageData> CommunityChat::execute(
    const ChatCommandContext& context, const DonateCoinsCommand& command) {
    const auto result = donations.donateCoins(context.player, command.quantity, context.actionsBlocked);
    return {CommunityMessages::donation(center, result)};
}

std::vector<ChatMessageData> CommunityChat::execute(
    const ChatCommandContext& context, const DonateItemCommand& command) {
    const auto item = shop_products::item(command.product);
    if (!item) {
        return {CommunityMessages::donation(center, {CommunityDonationError::UnsupportedItem, 0})};
    }
    const auto result = donations.donateItem(context.player, *item, command.quantity, context.actionsBlocked);
    return {CommunityMessages::donation(center, result, item)};
}

std::vector<ChatMessageData> CommunityChat::completeCheat(const CheatCommand& command) {
    if (command.action == CheatAction::CompleteAllBundles) {
        for (const auto bundle : community_bundle::all) center.complete(bundle);
    } else if (command.action == CheatAction::CompleteBundle) {
        const auto bundle = community_bundle::fromName(command.target);
        if (!bundle) return {CommunityMessages::message("Unknown bundle. Use crops, metals, animals, combat or money.")};
        center.complete(*bundle);
    } else {
        return {CommunityMessages::message("Unknown community center cheat.")};
    }
    return {CommunityMessages::message("Bundles updated. Rewards are granted only once per player.")};
}

std::vector<ChatMessageData> CommunityChat::locationMessages(const ChatCommandContext& context) {
    if (!CommunityDonations::isInsideCenter(context.player)) {
        visitors.erase(context.playerId);
        return {};
    }
    if (!visitors.insert(context.playerId).second) return {};
    return {
        CommunityMessages::message("Welcome! /center lists the bundles; /center metals shows the details."),
        CommunityMessages::message("/donate product amount | /donate_coins amount | /rewards")
    };
}

std::vector<ChatMessageData> CommunityChat::collectRewards(const ChatCommandContext& context) {
    auto messages = CommunityMessages::delivery(rewards.collect(context.player, context.playerName));
    if (center.finished() && victoryRecipients.insert(context.playerName).second) {
        messages.push_back(CommunityMessages::message(
            "Congratulations! All community center bundles are complete. You finished the game."));
    }
    return messages;
}

CommunityDonationReply CommunityChat::donate(const ChatCommandContext& context, const DonateDTO& command) {
    CommunityDonationResult result;
    std::optional<ItemId> item;
    if (command.kind == DonationKind::Coins) {
        result = donations.donateCoins(context.player, command.quantity, context.actionsBlocked);
    } else if (command.kind == DonationKind::Item) {
        const auto selected = context.player.getInventoryItems().find(command.slot);
        if (selected != context.player.getInventoryItems().end()) item = selected->second.item_id;
        result = donations.donateSlot(context.player, command.slot, command.quantity, context.actionsBlocked);
    } else {
        result.error = CommunityDonationError::UnsupportedItem;
    }
    return {{command.requestId, command.kind, command.slot, result.error, result.amount},
            {CommunityMessages::donation(center, result, item)}};
}

CommunityStateData CommunityChat::state(const ChatCommandContext& context) const {
    return snapshot.forPlayer(context.player, context.playerName, context.actionsBlocked);
}

void CommunityChat::onDisconnect(uint16_t playerId) noexcept { visitors.erase(playerId); }
