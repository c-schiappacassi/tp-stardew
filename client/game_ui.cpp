#include "game_ui.h"

#include "sdlGUI/render_info.h"
#include "sdlGUI/ui/sdl_input.h"


namespace {
    constexpr int WINDOW_W = 1920;
    constexpr int WINDOW_H = 1280;
    constexpr int CAMERA_ZOOM = 3;
    constexpr const char* WINDOW_TITLE = "Stardew Valley";
    constexpr const char* ASSETS_ROOT = ASSETS_DIR;
}

GameUI::GameUI(const std::string& local_player, const std::string& gameId, const ConstMapData& map_data):
        sdl(SDL_INIT_VIDEO),
        window(std::string(WINDOW_TITLE) + " | Codigo: " + gameId,   SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_W, WINDOW_H,  SDL_WINDOW_RESIZABLE),
        renderer(window, -1, SDL_RENDERER_ACCELERATED),
        // TODO: al asset manager le llega la ruta del minimap para cargar, por ahora hardcodeo en sprites_defs.h
        asset_manager(renderer, ASSETS_ROOT),
        cam(WINDOW_W, WINDOW_H, CAMERA_ZOOM),
        local_player(local_player),
        world(local_player, map_data, asset_manager),
        ui(local_player) {
    
    SDL_StartTextInput();
}

std::vector<std::shared_ptr<CommandDTO>> GameUI::poll_input() {
    std::vector<std::shared_ptr<CommandDTO>> out;
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (const auto event = sdl_input::translate(e)) {
            ui.dispatch(*event, out);
        }
    }
    ui.end_frame(out);
    return out;
}

bool GameUI::quit_requested() const { return ui.quit_requested(); }

void GameUI::update_camera(const State& state) {
    const SDL2pp::Point size = renderer.GetOutputSize();
    cam.set_viewport(size.x, size.y);

    if (const PlayerData* me = state.find_player(local_player)) {
        currentZone = me->currentZone;
        cam.set_world_size(world.world_size());
        cam.center_on(me->pos.getX(), me->pos.getY());
    }
}

void GameUI::update(const State& state, long long it) {
    world.update(state);
    update_camera(state);

    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();

    ui.update(state);

    RenderInfo ctx{renderer, asset_manager, state, cam, currentZone, it};
    world.draw(ctx);
    ui.draw(ctx);

    renderer.Present();
}
