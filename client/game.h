#pragma once

#include <string>

#include "game_ui.h"
#include "network.h"
#include "state.h"

class Game {
private:
    bool running = true;
    std::string player_name;
    NetworkManager net;
    State currentState;
    GameUI ui;

    // TODO: placeholder hasta que este la pantalla de Qt para crear/unirse a una partida.
    void connect_placeholder();

    void loopInternalLogic(long long it);

public:
    Game(const std::string& host, const std::string& port, const std::string& player_name);

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    void exe();
};