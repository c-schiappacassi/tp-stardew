#pragma once

#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "game/game_types.h"
#include "player/player.h"

class PlayerRecord {
private:
    uint16_t id;
    std::string name;
    std::shared_ptr<OutQueue> output;
    Player player;
    std::deque<ChatMessageData> chatMessages{};
    CommunityStateData community{};
    std::optional<CommunityDonationReceipt> lastDonation{};

public:
    PlayerRecord(uint16_t id, std::string name, Player player,std::shared_ptr<OutQueue> output = nullptr);

    uint16_t getId() const;
    const std::string& getName() const;
    bool isConnected() const;
    Player& getPlayer();
    const Player& getPlayer() const;

    void connect(const std::shared_ptr<OutQueue>& newOutput);
    void disconnect();
    void closeOutput();
    void enqueueMessage(const ChatMessageData& message);
    void enqueueMessages(const std::vector<ChatMessageData>& messages);
    void setCommunityState(CommunityStateData state);
    void setDonationReceipt(const CommunityDonationReceipt& receipt);
    CommunityStateData getCommunityData() const;
    void publishStatus(ServerStatus& status);
};
