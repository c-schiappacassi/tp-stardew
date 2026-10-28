#pragma once
#include <SDL2pp/SDL2pp.hh>
#include "asset_manager.h"
#include "camera.h"
#include "../state.h"

struct RenderInfo {
    SDL2pp::Renderer& r;
    AssetManager& assets;
    const State& state;
    const Camera& cam;
    Zone currentZone;
    long long it;
};