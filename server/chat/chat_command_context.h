#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "position.h"

class Player;

struct ChatCommandContext {
    uint16_t playerId;
    const std::string& playerName;
    Player& player;
    bool actionsBlocked;
    std::vector<Position> occupiedPositions{};
};
