#include "player_gifts.h"

#include <iomanip>
#include <sstream>
#include <string>

#include "commerce/shop_products.h"
#include "entities/entity_dimensions.h"
#include "player/sessions/player_roster.h"

namespace {
void reply(PlayerRecord& player, const std::string& text) { player.enqueueMessage({"Server", text}); }

bool giftable(ItemId item) {
    return item != ItemId::Pickaxe && item != ItemId::WateringCan && item != ItemId::Shovel && item != ItemId::Sword && item != ItemId::Bed && item != ItemId::BedSheet;
}

bool nearby(const Player& sender, const Player& recipient) {
    return sender.getZone() == recipient.getZone() && sender.getDungeonLevel() == recipient.getDungeonLevel() && sender.getInterior() == recipient.getInterior() &&
        sender.canReach(recipient.getPosition(), 2 * entity_dimensions::TILE_SIZE);
}

std::string description(const Stack& stack) {
    std::string result = std::to_string(stack.quantity) + " " + std::string{shop_products::name(stack.item_id)};
    switch (stack.qual) {
        case Quality::None: break;
        case Quality::Base: result += " (base)"; break;
        case Quality::Good: result += " (good)"; break;
        case Quality::Excellent: result += " (excellent)"; break;
        case Quality::Incredible: result += " (incredible)"; break;
        case Quality::Perfect: result += " (perfect)"; break;
    }
    return result;
}

std::string mention(const std::string& name) {
    if (name.find_first_of(" \"\\") == std::string::npos)
        return "@" + name;
    std::ostringstream result;
    result << '@' << std::quoted(name);
    return result.str();
}
}  // namespace

void PlayerGifts::execute(PlayerRoster& players, PlayerRecord& sender, const GiftCommand& command, bool actionsBlocked) {
    if (command.action == GiftAction::Cancel) {
        cancel(players, sender);
        return;
    }
    PlayerRecord* target = nullptr;
    for (auto& [id, record]: players) {
        if (record.getName() == command.player) {
            target = &record;
            break;
        }
    }
    if (!target || !target->isConnected()) {
        reply(sender, "The player does not exist or is offline.");
        return;
    }
    if (target->getId() == sender.getId()) {
        reply(sender, "You cannot give yourself a gift.");
        return;
    }
    if (command.action == GiftAction::Offer)
        offer(sender, *target, command.quantity, actionsBlocked);
    else
        respond(sender, *target, command.action == GiftAction::Accept, actionsBlocked);
}

void PlayerGifts::offer(PlayerRecord& sender, PlayerRecord& recipient, uint16_t quantity, bool actionsBlocked) {
    const auto& player = sender.getPlayer();
    if (actionsBlocked || !player.canAct() || !recipient.getPlayer().canAct()) {
        reply(sender, "Both players must be able to act to exchange gifts.");
        return;
    }
    if (!nearby(player, recipient.getPlayer())) {
        reply(sender, "Approach the player to offer a gift.");
        return;
    }
    const auto* selected = player.getSelectedItem();
    if (!selected) {
        reply(sender, "Select an item in your inventory before offering a gift.");
        return;
    }
    if (!giftable(selected->item_id)) {
        reply(sender, "Tools and beds cannot be given as gifts.");
        return;
    }
    if (quantity == 0 || quantity > selected->quantity) {
        reply(sender, "You do not have that amount in the selected slot.");
        return;
    }
    if (offers.contains(sender.getId())) {
        reply(sender, "You already have a pending gift. Wait for a response or use /cancel_gift.");
        return;
    }
    const Stack stack{selected->item_id, quantity, selected->qual};
    offers.emplace(sender.getId(), Offer{recipient.getId(), player.getSelectedSlot(), stack});
    reply(sender, "Gift offered to " + recipient.getName() + ": " + description(stack) + ". Waiting for a response.");
    reply(recipient, sender.getName() + " offers you " + description(stack) + ". Use /accept " + mention(sender.getName()) + " or /reject " + mention(sender.getName()) + ".");
}

void PlayerGifts::respond(PlayerRecord& recipient, PlayerRecord& sender, bool accepted, bool actionsBlocked) {
    const auto found = offers.find(sender.getId());
    if (found == offers.end() || found->second.recipientId != recipient.getId()) {
        reply(recipient, "You have no pending gift from that player.");
        return;
    }
    if (!accepted) {
        offers.erase(found);
        reply(sender, recipient.getName() + " rejected your gift.");
        reply(recipient, "You rejected the gift from " + sender.getName() + ".");
        return;
    }
    auto& source = sender.getPlayer();
    auto& destination = recipient.getPlayer();
    if (actionsBlocked || !source.canAct() || !destination.canAct()) {
        reply(recipient, "Both players must be able to act to exchange gifts.");
        return;
    }
    if (!nearby(source, destination)) {
        reply(recipient, "Approach the player to accept the gift.");
        return;
    }
    const auto offer = found->second;
    const auto& items = source.getInventoryItems();
    const auto item = items.find(offer.slot);
    if (item == items.end() || item->second.item_id != offer.stack.item_id || item->second.qual != offer.stack.qual || item->second.quantity < offer.stack.quantity) {
        offers.erase(found);
        reply(sender, "Your gift was cancelled because the offered item changed.");
        reply(recipient, "The gift was cancelled because the offered item is no longer available.");
        return;
    }
    if (!source.transferItemsTo(destination, offer.slot, offer.stack.quantity)) {
        reply(recipient, "Your inventory has no room for the gift. Make space and accept again, or reject it.");
        return;
    }
    offers.erase(found);
    reply(sender, recipient.getName() + " accepted your gift: " + description(offer.stack) + ".");
    reply(recipient, "You received " + description(offer.stack) + " from " + sender.getName() + ".");
}

void PlayerGifts::cancel(PlayerRoster& players, PlayerRecord& sender) {
    const auto found = offers.find(sender.getId());
    if (found == offers.end()) {
        reply(sender, "You have no outgoing gift to cancel.");
        return;
    }
    auto* recipient = players.find(found->second.recipientId);
    offers.erase(found);
    reply(sender, "You cancelled your gift.");
    if (recipient && recipient->isConnected())
        reply(*recipient, sender.getName() + " cancelled the gift offer.");
}

void PlayerGifts::onDisconnect(uint16_t playerId) noexcept {
    std::erase_if(offers, [playerId](const auto& entry) { return entry.first == playerId || entry.second.recipientId == playerId; });
}

void PlayerGifts::clear() noexcept { offers.clear(); }
