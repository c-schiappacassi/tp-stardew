#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "dtos.h"
#include "input_translator.h"
#include "sdlGUI/asset_manager.h"
#include "sdlGUI/camera.h"
#include "sdlGUI/world/world_view.h"
#include "state.h"

class GameUI {
private:
    SDL2pp::SDL sdl;
    SDL2pp::Window window;
    SDL2pp::Renderer renderer;
    AssetManager asset_manager;
    Camera cam;
    Zone currentZone = Zone::Farm;
    std::string local_player;
    WorldView world;
    InputTranslator input;

    void update_camera(const State& state);
public:
    GameUI(const std::string& local_player, const std::string& gameId, const ConstMapData& map_data);

    GameUI(const GameUI&) = delete;
    GameUI& operator=(const GameUI&) = delete;

    std::vector<std::shared_ptr<CommandDTO>> poll_input();
    bool quit_requested() const;

    void update(const State& state, long long it);
};
