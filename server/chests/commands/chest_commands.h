#pragma once

#include <cstdint>

#include "chest_action.h"

class Chest;
class PlayerManager;

class ChestCommands {
private:
    bool transfer(Chest& chest, PlayerManager& players, uint16_t playerId, const chest_action::Deposit& action) const;
    bool transfer(Chest& chest, PlayerManager& players, uint16_t playerId, const chest_action::Withdraw& action) const;

public:
    bool apply(Chest& chest, PlayerManager& players, uint16_t playerId, const ChestAction& action) const;
};
