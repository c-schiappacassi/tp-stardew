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
    std::map<std::string, PlayerView> player_views;

    void sync_views(const State& state) {
        for (const auto& entry : state.get_players()) {
            auto res = player_views.try_emplace(entry.first, entry.first, entry.second);
            if (!res.second) {
                res.first->second.update(entry.second);
            }
        }
        for (auto it = player_views.begin(); it != player_views.end();) {
            it = state.find_player(it->first) ? std::next(it) : player_views.erase(it);
        }
    }

public:
    explicit GameUI(const std::string& local_player)
        : sdl(SDL_INIT_VIDEO),
          window("Stardew Valley", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_RESIZABLE),
          renderer(window, -1, SDL_RENDERER_ACCELERATED),
          asset_manager(renderer, "common/assets"),
          cam(800, 600, 3),
          local_player(local_player) {}

    void update(const State& state, long long it) {
        sync_views(state);

        if (const PlayerData* me = state.find_player(local_player)) {
            const SDL2pp::Point size = renderer.GetOutputSize();
            cam.set_viewport(size.x, size.y);
            cam.center_on(me->pos.getX(), me->pos.getY());
        }

        RenderInfo ctx{renderer, asset_manager, state, cam, currentZone, it};

        renderer.SetDrawColor(55, 55, 55, 255); // TEMP hasta que este el mapa
        renderer.Clear();

        std::vector<std::pair<float, PlayerView*>> visible;
        for (auto& entry : player_views) {
            if (auto depth = entry.second.depth(ctx)) {
                visible.emplace_back(*depth, &entry.second);
            }
        }
        std::sort(visible.begin(), visible.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });
        for (auto& v : visible) {
            v.second->draw(ctx);
        }
        renderer.Present();
    }
};
