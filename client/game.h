#pragma once

#include <chrono>
#include "foo.h"
#include "network.h"
#include "inputTranslator.h"
#include "state.h"
#include "gameUI.h"

#define FPS 30

class Game
{
    private:
        bool running;
        long long it_interno = 0;
        InputTranslator translator;
        NetworkManager net;
        State currentState;
        GameUI ui;
        void loopInternalLogic(long long it);
    public:
        void exe();
};