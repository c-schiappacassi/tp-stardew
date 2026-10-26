#pragma once
#include <SDL2pp/SDL2pp.hh>
#include "state.h"
#include "sdlGUI/asset_manager.h"
#include "sdlGUI/camera.h"

// TODO
class GameUI {
private:
    SDL2pp::Renderer renderer;
    AssetManager asset_manager;
    Camera cam;

public:
    explicit GameUI();
    
    void update(const State&, long long it);
};
