#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>

#include "chat_commands.h"
#include "chat_command_parser.h"
#include "player/player_record.h"

class PlayerChat {
private:
    ChatServices services;

    static void reply(PlayerRecord& sender, const std::string& text);
    static void privateMessage(std::map<uint16_t, PlayerRecord>& players, PlayerRecord& sender, const std::string& text);
    void command(std::map<uint16_t, PlayerRecord>& players, PlayerRecord& sender,const std::string& text, bool dayEnded) const;
    void listCheats(PlayerRecord& sender) const;

public:
    static constexpr std::size_t MAX_LENGTH = 100;

    explicit PlayerChat(ChatServices services = {});
    bool interact(PlayerRecord& sender, Direction direction, bool dayEnded) const;
    void onDisconnect(uint16_t playerId) const noexcept;
    void handle(std::map<uint16_t, PlayerRecord>& players, uint16_t senderId,const std::string& text, bool dayEnded = false) const;
};
