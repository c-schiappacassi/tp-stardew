#pragma once

#include <SDL2pp/SDL2pp.hh>

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "sdlGUI/asset_manager.h"
#include "sdlGUI/camera.h"
#include "sdlGUI/render_info.h"
#include "sdlGUI/world/player_view.h"
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

public:
    explicit GameUI(const std::string& local_player)
        : sdl(SDL_INIT_VIDEO),
          window("Stardew Valley", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_RESIZABLE),
          renderer(window, -1, SDL_RENDERER_ACCELERATED),
          asset_manager(renderer, "common/assets"),
          cam(800, 600, 3),
          local_player(local_player),
          world(WorldView(local_player)) {}

    void update(const State& state, long long it) {
        if (const PlayerData* me = state.find_player(local_player)) {
            const SDL2pp::Point size = renderer.GetOutputSize();
            cam.set_viewport(size.x, size.y);
            cam.center_on(me->pos.getX(), me->pos.getY());
        }

        world.update(state);

        RenderInfo ctx{renderer, asset_manager, state, cam, currentZone, it};
        
        world.draw(ctx); // primero map/tiles del piso, despues entitys order by y, y por ultimo hud

        renderer.Present();
    }
};
