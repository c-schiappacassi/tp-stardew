#pragma once

#include <chrono>

class MovementRules;
class PlayerRoster;

class PlayerUpdates {
private:
    const MovementRules& movement;

public:
    explicit PlayerUpdates(const MovementRules& movement);

    void updateToolStates(PlayerRoster& players, std::chrono::steady_clock::time_point now) const;
    void advancePlayers(PlayerRoster& players, int normalStep) const;
};
