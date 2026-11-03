#pragma once

#include <chrono>
#include <cstdint>
#include <map>

#include "player/player_record.h"
#include "ports/movement_rules.h"

class PlayerUpdates {
private:
    const MovementRules& movement;

public:
    explicit PlayerUpdates(const MovementRules& movement);

    void updateToolStates(std::map<uint16_t, PlayerRecord>& players,std::chrono::steady_clock::time_point now) const;
    void advancePlayers(std::map<uint16_t, PlayerRecord>& players,int normalStep) const;
};