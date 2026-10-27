#pragma once

#include <chrono>
#include "foo.h"
#include "network.h"
#include "input_translator.h"
#include "state.h"
#include "game_ui.h"

class Game {
private:
    bool running = true;
    std::string player_name;
    InputTranslator translator;
    NetworkManager net;
    State currentState;
    GameUI ui;
    void loopInternalLogic(long long it);

public:
    Game(const std::string& host, const std::string& port, const std::string& player_name)
        : player_name(player_name), translator(player_name), net(host, port), ui(player_name) {}
        
    void exe();
};