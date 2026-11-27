#include "player_gifts.h"

#include "server_constants.h"

#include <iomanip>
#include <sstream>
#include <string>

#include "commerce/shop_products.h"
#include "entities/entity_dimensions.h"
#include "player/sessions/player_roster.h"

void PlayerGifts::reply(PlayerRecord& player, const std::string& text) { player.enqueueMessage({server_constants::SERVER, text}); }

bool PlayerGifts::giftable(ItemId item) {
    return item != ItemId::Pickaxe && item != ItemId::WateringCan && item != ItemId::Shovel && item != ItemId::Sword && item != ItemId::Bed && item != ItemId::BedSheet;
}

bool PlayerGifts::nearby(const Player& sender, const Player& recipient) {
    return sender.getZone() == recipient.getZone() && sender.getDungeonLevel() == recipient.getDungeonLevel() && sender.getInterior() == recipient.getInterior() &&
        sender.canReach(recipient.getPosition(), 2 * entity_dimensions::TILE_SIZE);
}

std::string PlayerGifts::description(const Stack& stack) {
    std::string result = std::to_string(stack.quantity) + server_constants::SPACE + std::string{shop_products::name(stack.item_id)};
    switch (stack.qual) {
        case Quality::None: break;
        case Quality::Base: result += server_constants::BASE_SUFFIX; break;
        case Quality::Good: result += server_constants::GOOD_SUFFIX; break;
        case Quality::Excellent: result += server_constants::EXCELLENT_SUFFIX; break;
        case Quality::Incredible: result += server_constants::INCREDIBLE_SUFFIX; break;
        case Quality::Perfect: result += server_constants::PERFECT_SUFFIX; break;
    }
    return result;
}

std::string PlayerGifts::mention(const std::string& name) {
    if (name.find_first_of(server_constants::QUOTING_CHARACTERS) == std::string::npos)
        return server_constants::AT_SIGN + name;
    std::ostringstream result;
    result << '@' << std::quoted(name);
    return result.str();
}

void PlayerGifts::execute(PlayerRoster& players, PlayerRecord& sender, const GiftCommand& command, bool actionsBlocked) {
    if (command.action == GiftAction::Cancel) {
        cancel(players, sender);
        return;
    }
    PlayerRecord* target = nullptr;
    for (auto& [id, record]: players) {
        if (record.isConnected() && record.getPlayer().getCharacterName() == command.player) {
            target = &record;
            break;
        }
    }
    if (!target) {
        reply(sender, server_constants::THE_PLAYER_DOES_NOT_EXIST_OR_IS_OFFLINE_2);
        return;
    }
    if (target->getId() == sender.getId()) {
        reply(sender, server_constants::YOU_CANNOT_GIVE_YOURSELF_A_GIFT);
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
        reply(sender, server_constants::BOTH_PLAYERS_MUST_BE_ABLE_TO_ACT_TO_EXCHANGE_GIFTS);
        return;
    }
    if (!nearby(player, recipient.getPlayer())) {
        reply(sender, server_constants::APPROACH_THE_PLAYER_TO_OFFER_A_GIFT);
        return;
    }
    const auto* selected = player.getSelectedItem();
    if (!selected) {
        reply(sender, server_constants::SELECT_AN_ITEM_IN_YOUR_INVENTORY_BEFORE_OFFERING_A_GIFT);
        return;
    }
    if (!giftable(selected->item_id)) {
        reply(sender, server_constants::TOOLS_AND_BEDS_CANNOT_BE_GIVEN_AS_GIFTS);
        return;
    }
    if (quantity == 0 || quantity > selected->quantity) {
        reply(sender, server_constants::YOU_DO_NOT_HAVE_THAT_AMOUNT_IN_THE_SELECTED_SLOT);
        return;
    }
    if (offers.contains(sender.getId())) {
        reply(sender, server_constants::YOU_ALREADY_HAVE_A_PENDING_GIFT_WAIT_FOR_A_RESPONSE_OR_USE);
        return;
    }
    const Stack stack{selected->item_id, quantity, selected->qual};
    offers.emplace(sender.getId(), Offer{recipient.getId(), player.getSelectedSlot(), stack});
    reply(sender, server_constants::GIFT_OFFERED_TO_PREFIX + recipient.getPlayer().getCharacterName() + server_constants::COLON_SPACE + description(stack) + server_constants::FIELD_WAITING_FOR_A_RESPONSE);
    reply(recipient, player.getCharacterName() + server_constants::OFFERS_YOU_SEPARATOR + description(stack) + server_constants::FIELD_USE_ACCEPT_PREFIX + mention(player.getCharacterName()) + server_constants::OR_REJECT_SEPARATOR + mention(player.getCharacterName()) + server_constants::DOT);
}

void PlayerGifts::respond(PlayerRecord& recipient, PlayerRecord& sender, bool accepted, bool actionsBlocked) {
    const auto found = offers.find(sender.getId());
    if (found == offers.end() || found->second.recipientId != recipient.getId()) {
        reply(recipient, server_constants::YOU_HAVE_NO_PENDING_GIFT_FROM_THAT_PLAYER);
        return;
    }
    if (!accepted) {
        offers.erase(found);
        reply(sender, recipient.getPlayer().getCharacterName() + server_constants::REJECTED_YOUR_GIFT_SUFFIX);
        reply(recipient, server_constants::YOU_REJECTED_THE_GIFT_FROM_PREFIX + sender.getPlayer().getCharacterName() + server_constants::DOT);
        return;
    }
    auto& source = sender.getPlayer();
    auto& destination = recipient.getPlayer();
    if (actionsBlocked || !source.canAct() || !destination.canAct()) {
        reply(recipient, server_constants::BOTH_PLAYERS_MUST_BE_ABLE_TO_ACT_TO_EXCHANGE_GIFTS);
        return;
    }
    if (!nearby(source, destination)) {
        reply(recipient, server_constants::APPROACH_THE_PLAYER_TO_ACCEPT_THE_GIFT);
        return;
    }
    const auto offer = found->second;
    const auto& items = source.getInventoryItems();
    const auto item = items.find(offer.slot);
    if (item == items.end() || item->second.item_id != offer.stack.item_id || item->second.qual != offer.stack.qual || item->second.quantity < offer.stack.quantity) {
        offers.erase(found);
        reply(sender, server_constants::YOUR_GIFT_WAS_CANCELLED_BECAUSE_THE_OFFERED_ITEM_CHANGED);
        reply(recipient, server_constants::THE_GIFT_WAS_CANCELLED_BECAUSE_THE_OFFERED_ITEM_IS_NO_LONGER_AVAILABLE);
        return;
    }
    if (!source.transferItemsTo(destination, offer.slot, offer.stack.quantity)) {
        reply(recipient, server_constants::YOUR_INVENTORY_HAS_NO_ROOM_FOR_THE_GIFT_MAKE_SPACE_AND_ACCEPT);
        return;
    }
    offers.erase(found);
    reply(sender, destination.getCharacterName() + server_constants::ACCEPTED_YOUR_GIFT_SEPARATOR + description(offer.stack) + server_constants::DOT);
    reply(recipient, server_constants::YOU_RECEIVED_PREFIX + description(offer.stack) + server_constants::FROM_SEPARATOR + source.getCharacterName() + server_constants::DOT);
}

void PlayerGifts::cancel(PlayerRoster& players, PlayerRecord& sender) {
    const auto found = offers.find(sender.getId());
    if (found == offers.end()) {
        reply(sender, server_constants::YOU_HAVE_NO_OUTGOING_GIFT_TO_CANCEL);
        return;
    }
    auto* recipient = players.find(found->second.recipientId);
    offers.erase(found);
    reply(sender, server_constants::YOU_CANCELLED_YOUR_GIFT);
    if (recipient && recipient->isConnected())
        reply(*recipient, sender.getPlayer().getCharacterName() + server_constants::CANCELLED_THE_GIFT_OFFER_SUFFIX);
}

void PlayerGifts::onDisconnect(uint16_t playerId) noexcept {
    std::erase_if(offers, [playerId](const auto& entry) { return entry.first == playerId || entry.second.recipientId == playerId; });
}

void PlayerGifts::clear() noexcept { offers.clear(); }
