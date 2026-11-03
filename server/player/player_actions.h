#pragma once

#include "game/command_router.h"
#include "ports/player_interactions.h"

class PlayerActions {
private:
    CommandRouter router;

public:
    explicit PlayerActions(PlayerInteractions& interactions);

    void execute(Player& player, const CommandDTO& command) const;
};