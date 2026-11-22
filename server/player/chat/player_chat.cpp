#include "player_chat.h"

#include <algorithm>
#include <exception>
#include <utility>
#include <vector>

#include "chat/chat_command_context.h"
#include "chat/services/cheat_chat_service.h"
#include "chat/services/community_chat_service.h"
#include "chat/services/merchant_chat_service.h"
#include "player/sessions/player_roster.h"

PlayerChat::PlayerChat(PlayerManager& manager, ChatServices services):
        manager(manager), services(std::move(services)) {}

bool PlayerChat::interact(PlayerRecord& sender, Direction direction) const {
    if (!services.merchants || !sender.getPlayer().canAct())
        return false;
    const ChatCommandContext context{sender.getId(), sender.getName(), manager, false};
    try {
        const auto responses = services.merchants->interact(context, direction);
        if (!responses)
            return false;
        sender.enqueueMessages(*responses);
    } catch (const std::exception&) {
        reply(sender, "Could not start the conversation. Try again.");
    }
    return true;
}

void PlayerChat::onDisconnect(uint16_t playerId) noexcept {
    gifts.onDisconnect(playerId);
    if (services.merchants)
        services.merchants->onDisconnect(playerId);
    if (services.cheats)
        services.cheats->onDisconnect(playerId);
    if (services.community)
        services.community->onDisconnect(playerId);
}

void PlayerChat::clearGifts() noexcept { gifts.clear(); }

void PlayerChat::reply(PlayerRecord& sender, const std::string& text) {
    sender.enqueueMessage({"Server", text});
}

void PlayerChat::collectCommunityRewards(PlayerRecord& record, bool actionsBlocked) const {
    if (!services.community)
        return;
    const ChatCommandContext context{record.getId(), record.getName(), manager, actionsBlocked};
    if (record.isConnected()) {
        const auto welcome = services.community->locationMessages(context);
        record.enqueueMessages(welcome);
    }
    const auto messages = services.community->collectRewards(context);
    record.enqueueMessages(messages);
    record.setCommunityState(services.community->state(context));
}

void PlayerChat::donate(PlayerRecord& record, const DonateDTO& command, bool actionsBlocked) const {
    if (!services.community) {
        record.setDonationReceipt({command.requestId, command.kind, command.slot,CommunityDonationError::CannotAct, 0});
        reply(record, "The community center is not available.");
        return;
    }
    const ChatCommandContext context{record.getId(), record.getName(), manager, actionsBlocked};
    const auto response = services.community->donate(context, command);
    record.setDonationReceipt(response.receipt);
    record.enqueueMessages(response.messages);
}

void PlayerChat::privateMessage(PlayerRoster& players, PlayerRecord& sender,const std::string& text) {
    PlayerRecord* recipient = nullptr;
    std::size_t body = std::string::npos;
    const auto separator = text.find(' ');
    if (separator == std::string::npos || separator == 1) {
        reply(sender, "Usage: @username message");
        return;
    }
    for (auto& [id, record]: players) {
        const auto end = 1 + record.getName().size();
        if (text.compare(1, record.getName().size(), record.getName()) == 0 &&
            (end == text.size() || (end < text.size() && text[end] == ' ')) &&
            (!recipient || record.getName().size() > recipient->getName().size())) {
            recipient = &record;
            body = text.find_first_not_of(' ', end);
        }
    }
    if (recipient && body == std::string::npos) {
        reply(sender, "Usage: @username message");
        return;
    }
    if (!recipient || !recipient->isConnected()) {
        reply(sender, "The player does not exist or is offline");
        return;
    }
    recipient->enqueueMessage({sender.getName(), text});
    if (recipient->getId() != sender.getId())
        sender.enqueueMessage({sender.getName(), text});
}

void PlayerChat::handle(PlayerRoster& players, uint16_t senderId, const std::string& text,bool actionsBlocked) {
    auto* senderRecord = players.find(senderId);
    if (!senderRecord || !senderRecord->isConnected())
        return;
    auto& sender = *senderRecord;
    if (text.size() > MAX_LENGTH) {
        reply(sender, "The message cannot exceed 100 characters");
        return;
    }
    if (std::any_of(text.begin(), text.end(),
                    [](unsigned char ch) { return ch < 32 || ch == 127; })) {
        reply(sender, "The message contains control characters");
        return;
    }
    const auto first = text.find_first_not_of(' ');
    if (first == std::string::npos)
        return;
    const auto last = text.find_last_not_of(' ');
    const auto message = text.substr(first, last - first + 1);
    if (message.front() == '@') {
        privateMessage(players, sender, message);
        return;
    }
    if (message.front() == '/') {
        command(players, sender, message, actionsBlocked);
        return;
    }
    for (auto& [id, record]: players) {
        if (record.isConnected())
            record.enqueueMessage({sender.getName(), message});
    }
}
