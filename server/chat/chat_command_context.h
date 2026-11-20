#pragma once

#include <cstdint>
#include <string>

class Player;

struct ChatCommandContext {
    uint16_t playerId;
    const std::string& playerName;
    Player& player;
    bool actionsBlocked;
};
