#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "position.h"

class PlayerManager;

struct ChatCommandContext {
    uint16_t playerId;
    const std::string& playerName;
    PlayerManager& players;
    bool actionsBlocked;
    std::vector<Position> occupiedPositions{};
};
