#pragma once

#include <cstdint>
#include <map>

#include "chat/commands/gift_command.h"

#include "dtos.h"

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

public:
    void execute(PlayerRoster& players, PlayerRecord& sender, const GiftCommand& command, bool actionsBlocked);
    void onDisconnect(uint16_t playerId) noexcept;
    void clear() noexcept;
};
