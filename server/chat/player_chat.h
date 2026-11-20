#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>

#include "chat_services.h"
#include "protocol_types.h"

struct PlayerRecord;
struct DonateDTO;

class PlayerChat {
private:
    ChatServices services;

    static void reply(PlayerRecord& sender, const std::string& text);
    static void privateMessage(std::map<uint16_t, PlayerRecord>& players, PlayerRecord& sender, const std::string& text);
    void command(std::map<uint16_t, PlayerRecord>& players, PlayerRecord& sender,const std::string& text, bool actionsBlocked) const;
    void listCheats(PlayerRecord& sender) const;

public:
    static constexpr std::size_t MAX_LENGTH = 100;

    explicit PlayerChat(ChatServices services = {});
    bool interact(PlayerRecord& sender, Direction direction, bool actionsBlocked) const;
    void onDisconnect(uint16_t playerId) const noexcept;
    void collectCommunityRewards(PlayerRecord& player, bool actionsBlocked = false) const;
    void donate(PlayerRecord& player, const DonateDTO& command, bool actionsBlocked) const;
    void handle(std::map<uint16_t, PlayerRecord>& players, uint16_t senderId,const std::string& text, bool actionsBlocked = false) const;
};
