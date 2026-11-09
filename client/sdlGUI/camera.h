#pragma once

#include <SDL2pp/SDL2pp.hh>

class Camera {
private:
    static constexpr float TEXELS_PER_UNIT = 1.0f;

    int center_x = 0;
    int center_y = 0;
    int viewport_w;
    int viewport_h;
    int zoom;

    static int to_texels(int world_units) {
        return static_cast<int>(static_cast<float>(world_units) * TEXELS_PER_UNIT);
    }

    static int floor_div(int a, int b) {
        int q = a / b;
        return (a % b != 0 && ((a < 0) != (b < 0))) ? q - 1 : q;
    }

public:
    explicit Camera(int viewport_w, int viewport_h, int zoom): viewport_w(viewport_w), viewport_h(viewport_h), zoom(zoom) {}

    void set_viewport(int w, int h) {
        viewport_w = w;
        viewport_h = h;
    }

    void center_on(int world_x, int world_y) {
        center_x = world_x;
        center_y = world_y;
    }

    SDL2pp::Point world_to_screen(int world_x, int world_y) const {
        return SDL2pp::Point(viewport_w / 2 + (to_texels(world_x) - to_texels(center_x)) * zoom, viewport_h / 2 + (to_texels(world_y) - to_texels(center_y)) * zoom);
    }

    SDL2pp::Point screen_to_world(int screen_x, int screen_y) const {
        return SDL2pp::Point(center_x + floor_div(screen_x - viewport_w / 2, zoom), center_y + floor_div(screen_y - viewport_h / 2, zoom));
    }

    bool is_visible(int world_x, int world_y, int radius_px) const {
        const SDL2pp::Point p = world_to_screen(world_x, world_y);
        return p.x + radius_px >= 0 && p.x - radius_px <= viewport_w && p.y + radius_px >= 0 && p.y - radius_px <= viewport_h;
    }

    int get_zoom() const { return zoom; }
};