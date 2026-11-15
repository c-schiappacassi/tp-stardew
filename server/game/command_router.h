#pragma once

#include "dtos.h"

class Player;
class PlayerInteractions;

class CommandRouter {
private:
    PlayerInteractions& interactions;

public:
    explicit CommandRouter(PlayerInteractions& interactions);
    void dispatch(Player& player, const CommandDTO& command, uint16_t playerId) const;
};
