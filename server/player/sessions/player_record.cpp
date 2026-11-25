#include "player_record.h"

#include "server_constants.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>

PlayerRecord::PlayerRecord(uint16_t id, std::string name, Player player,std::shared_ptr<OutQueue> output):
        id(id), name(std::move(name)), output(std::move(output)), player(std::move(player)) {}

uint16_t PlayerRecord::getId() const { return id; }

const std::string& PlayerRecord::getName() const { return name; }

bool PlayerRecord::isConnected() const { return output != nullptr; }

Player& PlayerRecord::getPlayer() { return player; }

const Player& PlayerRecord::getPlayer() const { return player; }

void PlayerRecord::connect(const std::shared_ptr<OutQueue>& newOutput) {
    if (!newOutput)
        throw std::invalid_argument(server_constants::A_CONNECTED_PLAYER_NEEDS_AN_OUTPUT);
    output = newOutput;
    player.setMovement(Direction::None);
    player.closeChest();
}

void PlayerRecord::disconnect() {
    output.reset();
    chatMessages.clear();
    lastDonation.reset();
    player.setMovement(Direction::None);
    player.closeChest();
}

void PlayerRecord::closeOutput() {
    if (!output)
        return;
    try {
        output->close();
    } catch (const std::runtime_error&) {

    }
}

void PlayerRecord::enqueueMessage(const ChatMessageData& message) {
    chatMessages.push_back(message);
}

void PlayerRecord::enqueueMessages(const std::vector<ChatMessageData>& messages) {
    chatMessages.insert(chatMessages.end(), messages.begin(), messages.end());
}

void PlayerRecord::setCommunityState(CommunityStateData state) { community = std::move(state); }

void PlayerRecord::setDonationReceipt(const CommunityDonationReceipt& receipt) {
    lastDonation = receipt;
}

CommunityStateData PlayerRecord::getCommunityData() const {
    auto data = community;
    data.lastDonation = lastDonation;
    return data;
}

void PlayerRecord::publishStatus(ServerStatus& status) {
    if (!isConnected())
        return;
    const auto messageCount = std::min(
            chatMessages.size(), static_cast<std::size_t>(std::numeric_limits<uint16_t>::max()));
    const auto messagesEnd = chatMessages.begin() + static_cast<std::ptrdiff_t>(messageCount);
    status.localPlayer.newMessages.assign(chatMessages.begin(), messagesEnd);
    try {
        output->push(status);
        chatMessages.erase(chatMessages.begin(), messagesEnd);
    } catch (const ClosedQueue&) {

    }
}
