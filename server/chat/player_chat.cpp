#include "player_chat.h"

#include <algorithm>
#include <exception>
#include <utility>
#include <vector>

#include "chat_command_context.h"
#include "player/player_record.h"
#include "services/cheat_chat_service.h"
#include "services/community_chat_service.h"
#include "services/merchant_chat_service.h"

PlayerChat::PlayerChat(ChatServices services): services(std::move(services)) {}

bool PlayerChat::interact(PlayerRecord& sender, Direction direction, bool actionsBlocked) const {
    if (!services.merchants || actionsBlocked || !sender.player.canAct() ||
        direction > Direction::Right) return false;
    if (direction == Direction::None) direction = sender.player.getPublicData().dir;
    const ChatCommandContext context{sender.id, sender.name, sender.player, actionsBlocked};
    try {
        const auto responses = services.merchants->interact(context, direction);
        if (!responses) return false;
        sender.chatMessages.insert(sender.chatMessages.end(), responses->begin(), responses->end());
    } catch (const std::exception&) {
        reply(sender, "Could not start the conversation. Try again.");
    }
    return true;
}

void PlayerChat::onDisconnect(uint16_t playerId) const noexcept {
    if (services.merchants) services.merchants->onDisconnect(playerId);
    if (services.cheats) services.cheats->onDisconnect(playerId);
    if (services.community) services.community->onDisconnect(playerId);
}

void PlayerChat::reply(PlayerRecord& sender, const std::string& text) {
    sender.chatMessages.push_back({"Server", text});
}

void PlayerChat::collectCommunityRewards(PlayerRecord& record, bool actionsBlocked) const {
    if (!services.community) return;
    const ChatCommandContext context{record.id, record.name, record.player, actionsBlocked};
    if (record.connected) {
        const auto welcome = services.community->locationMessages(context);
        record.chatMessages.insert(record.chatMessages.end(), welcome.begin(), welcome.end());
    }
    const auto messages = services.community->collectRewards(context);
    record.chatMessages.insert(record.chatMessages.end(), messages.begin(), messages.end());
    record.community = services.community->state(context);
}

void PlayerChat::donate(PlayerRecord& record, const DonateDTO& command, bool actionsBlocked) const {
    if (!services.community) {
        record.lastDonation = {command.requestId, command.kind, command.slot,
                               CommunityDonationError::CannotAct, 0};
        reply(record, "The community center is not available.");
        return;
    }
    const ChatCommandContext context{record.id, record.name, record.player, actionsBlocked};
    const auto response = services.community->donate(context, command);
    record.lastDonation = response.receipt;
    record.chatMessages.insert(record.chatMessages.end(), response.messages.begin(), response.messages.end());
}

void PlayerChat::privateMessage(std::map<uint16_t, PlayerRecord>& players,PlayerRecord& sender, const std::string& text) {
    PlayerRecord* recipient = nullptr;
    std::size_t body = std::string::npos;
    const auto separator = text.find(' ');
    if (separator == std::string::npos || separator == 1) {
        reply(sender, "Usage: @username message");
        return;
    }
    for (auto& [id, record] : players) {
        const auto end = 1 + record.name.size();
        if (text.compare(1, record.name.size(), record.name) == 0 &&
            (end == text.size() || (end < text.size() && text[end] == ' ')) &&
            (!recipient || record.name.size() > recipient->name.size())) {
            recipient = &record;
            body = text.find_first_not_of(' ', end);
        }
    }
    if (recipient && body == std::string::npos) {
        reply(sender, "Usage: @username message");
        return;
    }
    if (!recipient || !recipient->connected || !recipient->output) {
        reply(sender, "The player does not exist or is offline");
        return;
    }
    recipient->chatMessages.push_back({sender.name, text});
    if (recipient->id != sender.id) sender.chatMessages.push_back({sender.name, text});
}

void PlayerChat::handle(std::map<uint16_t, PlayerRecord>& players, uint16_t senderId,
                        const std::string& text, bool actionsBlocked) const {
    const auto senderIt = players.find(senderId);
    if (senderIt == players.end() || !senderIt->second.connected || !senderIt->second.output) return;
    auto& sender = senderIt->second;
    if (text.size() > MAX_LENGTH) {
        reply(sender, "The message cannot exceed 100 characters");
        return;
    }
    if (std::any_of(text.begin(), text.end(), [](unsigned char ch) {
            return ch < 32 || ch == 127;
        })) {
        reply(sender, "The message contains control characters");
        return;
    }
    const auto first = text.find_first_not_of(' ');
    if (first == std::string::npos) return;
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
    for (auto& [id, record] : players) {
        if (record.connected && record.output) record.chatMessages.push_back({sender.name, message});
    }
}
