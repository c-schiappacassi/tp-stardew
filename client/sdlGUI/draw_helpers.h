#pragma once

#include <cmath>
#include <optional>
#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include <algorithm>

#include "render_info.h"
#include "utils.h"
#include "sprite.h"
#include "position.h"

inline SDL2pp::Rect rect_centered_on(const SDL2pp::Point& center, int w, int h) {
    return SDL2pp::Rect(center.x - w / 2, center.y - h / 2, w, h);
}

inline bool on_screen(const RenderInfo& ctx, int world_x, int world_y, int radius_world) {
    return ctx.cam.is_visible(world_x, world_y, radius_world * ctx.cam.get_zoom());
}

inline bool on_screen(const RenderInfo& ctx, const Position& pos, int radius_world) {
    return on_screen(ctx, pos.getX(), pos.getY(), radius_world);
}

inline std::optional<float> depth_if_visible(const RenderInfo& ctx, const Position& pos, int radius_world, int y_offset = 0) {
    if (!on_screen(ctx, pos, radius_world)) {
        return std::nullopt;
    }
    return static_cast<float>(static_cast<int>(pos.getY()) + y_offset);
}

inline constexpr int BOB_AMPLITUDE = 2;
inline constexpr double BOB_SPEED = 0.15;

inline int bob_offset(const RenderInfo& ctx, int amplitude = BOB_AMPLITUDE, double speed = BOB_SPEED, double phase = 0.0) {
    return static_cast<int>(std::lround(std::sin(static_cast<double>(ctx.it) * speed + phase) * amplitude));
}

inline SDL2pp::Rect scaled_rect(const SDL2pp::Rect& area, const Sprite& s, bool cover = false) {
    const int sw = std::max(1, s.get_width()), sh = std::max(1, s.get_height());
    int w = area.w, h = area.h;
    if (cover) {
        const double k = std::max(static_cast<double>(area.w) / sw, static_cast<double>(area.h) / sh);
        w = static_cast<int>(std::ceil(sw * k));
        h = static_cast<int>(std::ceil(sh * k));
    } else if (static_cast<long long>(area.w) * sh <= static_cast<long long>(area.h) * sw) {
        h = static_cast<int>(static_cast<long long>(sh) * area.w / sw);
    } else {
        w = static_cast<int>(static_cast<long long>(sw) * area.h / sh);
    }
    w = std::max(1, w);
    h = std::max(1, h);
    return SDL2pp::Rect(area.x + (area.w - w) / 2, area.y + (area.h - h) / 2, w, h);
}

template <typename Color>
SDL_Color to_sdl(const Color& color) {
    return SDL_Color{
        static_cast<Uint8>(color.r),
        static_cast<Uint8>(color.g),
        static_cast<Uint8>(color.b),
        255
    };
}