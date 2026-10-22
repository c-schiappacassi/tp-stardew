#pragma once
#include <SDL2pp/SDL2pp.hh>
#include "state.h"

// TODO
class GameUI {
private:
    SDL2pp::Renderer& renderer;

public:
    explicit GameUI(SDL2pp::Renderer& renderer) : renderer(renderer) {}

    void update(const State&, long long) {

    }
};
