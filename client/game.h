#pragma once

#include <string>

#include "game_ui.h"
#include "network.h"
#include "state.h"

class Game {
private:
    bool running = true;
    std::string player_name;
    NetworkManager& net;
    State currentState;
    GameUI ui;

    void loopInternalLogic(long long it);

public:
    Game(NetworkManager& net, const std::string& player_name, const ServerStatus& initialStatus);

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    void exe();
};
