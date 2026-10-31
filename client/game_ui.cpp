#include "game_ui.h"

#include "sdlGUI/render_info.h"

namespace {
constexpr int WINDOW_W = 1280;
constexpr int WINDOW_H = 720;
constexpr int CAMERA_ZOOM = 3;
constexpr const char* WINDOW_TITLE = "Stardew Valley";
constexpr const char* ASSETS_ROOT = "common/assets";
}

GameUI::GameUI(const std::string& local_player):
        sdl(SDL_INIT_VIDEO),
        window(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_W, WINDOW_H,
               SDL_WINDOW_RESIZABLE),
        renderer(window, -1, SDL_RENDERER_ACCELERATED),
        asset_manager(renderer, ASSETS_ROOT),
        cam(WINDOW_W, WINDOW_H, CAMERA_ZOOM),
        local_player(local_player),
        world(local_player),
        input(local_player, [this](int x, int y) { return pick_target(x, y); }) {}

std::vector<std::shared_ptr<CommandDTO>> GameUI::poll_input() { return input.poll(); }

bool GameUI::quit_requested() const { return input.quit_requested(); }

std::optional<ClickTarget> GameUI::pick_target(int window_x, int window_y) {
    const SDL2pp::Point out = renderer.GetOutputSize();
    const SDL2pp::Point win = window.GetSize();
    if (win.x == 0 || win.y == 0) {
        return std::nullopt;
    }
    const int sx = window_x * out.x / win.x;
    const int sy = window_y * out.y / win.y;
    return world.pick_target(cam, sx, sy);
}

void GameUI::update_camera(const State& state) {
    const SDL2pp::Point size = renderer.GetOutputSize();
    cam.set_viewport(size.x, size.y);

    if (const PlayerData* me = state.find_player(local_player)) {
        currentZone = me->zone;
        cam.center_on(me->pos.getX(), me->pos.getY());
    }
}

void GameUI::update(const State& state, long long it) {
    update_camera(state);

    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();

    world.update(state);

    RenderInfo ctx{renderer, asset_manager, state, cam, currentZone, it};
    world.draw(ctx);

    renderer.Present();
}