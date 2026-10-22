#pragma once

#include <chrono>
#include "foo.h"
#include "network.h"
#include "input_translator.h"
#include "state.h"
#include "game_ui.h"

class Game
{
    private:
        bool running;
        long long it_interno = 0;
        // TODO:TBD si gameUI no deberia ser el dueño de translator
        InputTranslator translator;
        NetworkManager net;
        State currentState;
        GameUI ui;
        void loopInternalLogic(long long it);
    public:
        void exe();
};