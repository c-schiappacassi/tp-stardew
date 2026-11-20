#include "community_donations.h"

#include <algorithm>
#include <vector>

#include "player/player.h"

CommunityDonations::CommunityDonations(CommunityCenter& center) : center(center) {}

bool CommunityDonations::isInsideCenter(const Player& player) {
    return player.getZone() == Zone::InsideHouse && player.getInterior() &&
        player.getInterior()->interior_id == InteriorId::ComunityCenter;
}

CommunityDonationError CommunityDonations::validate(const Player& player, uint32_t amount, bool actionsBlocked) {
    if (amount == 0) return CommunityDonationError::InvalidAmount;
    if (actionsBlocked || !player.canAct()) return CommunityDonationError::CannotAct;
    if (!isInsideCenter(player)) return CommunityDonationError::OutsideCenter;
    return CommunityDonationError::None;
}

CommunityDonationResult CommunityDonations::donateItem(
    Player& player, ItemId item, uint32_t amount, bool actionsBlocked) {
    const auto error = validate(player, amount, actionsBlocked);
    if (error != CommunityDonationError::None) return {error, 0};
    const auto bundle = community_bundle::forItem(item);
    if (!bundle) return {CommunityDonationError::UnsupportedItem, 0};
    const auto accepted = std::min(amount, center.remaining(item));
    if (accepted == 0) return {CommunityDonationError::AlreadyComplete, 0};

    uint32_t missing = accepted;
    std::vector<std::pair<uint8_t, uint16_t>> selected;
    for (const auto& [slot, stack] : player.getInventoryItems()) {
        if (stack.item_id != item || missing == 0) continue;
        if (!community_bundle::acceptsQuality(item, stack.qual)) continue;
        const auto quantity = static_cast<uint16_t>(std::min<uint32_t>(missing, stack.quantity));
        selected.emplace_back(slot, quantity);
        missing -= quantity;
    }
    if (missing != 0) return {CommunityDonationError::MissingItems, 0};
    if (!player.removeItems(selected)) return {CommunityDonationError::InventoryChanged, 0};
    center.donate(item, accepted);
    return {CommunityDonationError::None, accepted};
}

CommunityDonationResult CommunityDonations::donateCoins(Player& player, uint32_t amount, bool actionsBlocked) {
    const auto error = validate(player, amount, actionsBlocked);
    if (error != CommunityDonationError::None) return {error, 0};
    const auto accepted = std::min(amount, center.remainingCoins());
    if (accepted == 0) return {CommunityDonationError::AlreadyComplete, 0};
    if (!player.spendGold(accepted)) return {CommunityDonationError::MissingCoins, 0};
    center.donateCoins(accepted);
    return {CommunityDonationError::None, accepted};
}

CommunityDonationResult CommunityDonations::donateSlot(
    Player& player, uint8_t slot, uint32_t amount, bool actionsBlocked) {
    const auto error = validate(player, amount, actionsBlocked);
    if (error != CommunityDonationError::None) return {error, 0};
    const auto selected = player.getInventoryItems().find(slot);
    if (selected == player.getInventoryItems().end()) return {CommunityDonationError::InvalidSlot, 0};
    const auto& stack = selected->second;
    if (!community_bundle::forItem(stack.item_id)) return {CommunityDonationError::UnsupportedItem, 0};
    const auto accepted = std::min(amount, center.remaining(stack.item_id));
    if (accepted == 0) return {CommunityDonationError::AlreadyComplete, 0};
    if (stack.quantity < accepted || !community_bundle::acceptsQuality(stack.item_id, stack.qual)) {
        return {CommunityDonationError::MissingItems, 0};
    }
    const auto item = stack.item_id;
    if (!player.removeItems({{slot, static_cast<uint16_t>(accepted)}})) {
        return {CommunityDonationError::InventoryChanged, 0};
    }
    center.donate(item, accepted);
    return {CommunityDonationError::None, accepted};
}
