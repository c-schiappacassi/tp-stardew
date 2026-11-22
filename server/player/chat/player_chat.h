#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "chat/chat_services.h"

#include "protocol_types.h"

class PlayerManager;
class PlayerRecord;
class PlayerRoster;
struct DonateDTO;

class PlayerChat {
private:
    PlayerManager& manager;
    ChatServices services;

    static void reply(PlayerRecord& sender, const std::string& text);
    static void privateMessage(PlayerRoster& players, PlayerRecord& sender,const std::string& text);
    void command(const PlayerRoster& players, PlayerRecord& sender, const std::string& text,bool actionsBlocked) const;
    void listCheats(PlayerRecord& sender) const;

public:
    static constexpr std::size_t MAX_LENGTH = 100;

    explicit PlayerChat(PlayerManager& manager, ChatServices services = {});
    bool interact(PlayerRecord& sender, Direction direction) const;
    void onDisconnect(uint16_t playerId) const noexcept;
    void collectCommunityRewards(PlayerRecord& player, bool actionsBlocked = false) const;
    void donate(PlayerRecord& player, const DonateDTO& command, bool actionsBlocked) const;
    void handle(PlayerRoster& players, uint16_t senderId, const std::string& text,bool actionsBlocked = false) const;
};
