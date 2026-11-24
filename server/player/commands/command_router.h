#pragma once

#include "dtos.h"
#include "player_action.h"

class DayCycle;
class PlayerChat;
class PlayerManager;
class PlayerInteractions;
class PlayerRecord;
class PlayerRoster;

class CommandRouter {
private:
    PlayerManager& manager;
    PlayerRoster& roster;
    DayCycle& day;
    PlayerChat& chat;
    PlayerInteractions& interactions;

    bool handleSocialCommand(PlayerRecord& record, const CommandDTO& command, bool blocked) const;
    bool handleDayCommand(PlayerRecord& record, const CommandDTO& command) const;
    void interact(PlayerRecord& record, Direction requested) const;

public:
    CommandRouter(PlayerManager& manager, PlayerRoster& roster, DayCycle& day, PlayerChat& chat,PlayerInteractions& interactions);
    PlayerActionResult apply(uint16_t playerId, const PlayerAction& action) const;
    void dispatch(PlayerRecord& record, const CommandDTO& command, bool gameCompleted) const;
};
