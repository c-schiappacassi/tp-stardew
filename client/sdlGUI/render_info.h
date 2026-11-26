#pragma once
#include <SDL2pp/SDL2pp.hh>

#include "../state.h"

#include "asset_handling/asset_manager.h"
#include "audio/audio.h"
#include "camera.h"

struct RenderInfo {
    SDL2pp::Renderer& r;
    AssetManager& assets;
    const State& state;
    const Camera& cam;
    Zone currentZone;
    long long it;
    Audio& audio;
};