#pragma once

#include <algorithm>
#include <array>
#include <cstddef>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../../clip_player.h"
#include "ui_box.h"
#include "../../utils.h"

class DancingJunimoView {
private:
    static constexpr int BASE_SCALE = 5;

    static constexpr double HOP_RATIO = 0.20;
    static constexpr std::array<int, 8> SWAY{0, -1, -1, 0, 0, 1, 1, 0};
    static constexpr double SWAY_RATIO = 0.08;

    double nx;
    double ny;
    SDL_Color tint;
    int tier;
    JunimoState dance;
    double appear_ms;
    bool mirrored;

    ClipPlayer clip_player;
    std::size_t last_index = 0;

public:
    DancingJunimoView(double nx, double ny, const SDL_Color& tint, int tier, JunimoState dance, double appear_ms, bool mirrored):
            nx(nx), ny(ny), tint(tint), tier(tier), dance(dance), appear_ms(appear_ms), mirrored(mirrored) {}

    double depth() const { return ny; }

    void draw(const RenderInfo& ctx, const UiBox& screen, double scene_ms) {
        if (scene_ms < appear_ms) {
            return;
        }

        const AnimatedFrame* frame = clip_player.frame(ctx, ClipId{SpriteId::Junimo, Action{dance}, Direction::None});
        if (!frame) {
            return;
        }

        const std::size_t index = clip_player.frame_index();
        if (index < last_index) {
            mirrored = !mirrored;
        }
        last_index = index;

        const Sprite& sprite = frame->get_sprite();
        const bool flip = frame->is_flipped() != mirrored;
        const int k = std::max(1, BASE_SCALE * screen.get_unit() + tier);

        const bool arms_up = (index % 2) == 0;
        const int hop = arms_up ? static_cast<int>(sprite.get_height() * k * HOP_RATIO) : 0;
        const int sway = static_cast<int>(SWAY[index % SWAY.size()] * sprite.get_height() * k * SWAY_RATIO);

        const int feet_x = static_cast<int>(nx * screen.get_rect().w);
        const int feet_y = static_cast<int>(ny * screen.get_rect().h);

        sprite.draw(ctx, SDL2pp::Point(feet_x + sway, feet_y - hop), {.flip = flip, .tint = tint, .scale = k});
    }
};
