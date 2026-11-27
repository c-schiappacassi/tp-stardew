#pragma once

#include <cstdint>
#include <map>
#include <string>

#include "chat/commands/gift_command.h"

#include "dtos.h"

class Player;
class PlayerRecord;
class PlayerRoster;

class PlayerGifts {
private:
    struct Offer {
        uint16_t recipientId;
        uint8_t slot;
        Stack stack;
    };

    std::map<uint16_t, Offer> offers;

    void offer(PlayerRecord& sender, PlayerRecord& recipient, uint16_t quantity, bool actionsBlocked);
    void respond(PlayerRecord& recipient, PlayerRecord& sender, bool accepted, bool actionsBlocked);
    void cancel(PlayerRoster& players, PlayerRecord& sender);

    static void reply(PlayerRecord& player, const std::string& text);
    static bool giftable(ItemId item);
    static bool nearby(const Player& sender, const Player& recipient);
    static std::string description(const Stack& stack);
    static std::string mention(const std::string& name);

public:
    void execute(PlayerRoster& players, PlayerRecord& sender, const GiftCommand& command, bool actionsBlocked);
    void onDisconnect(uint16_t playerId) noexcept;
    void clear() noexcept;
};
