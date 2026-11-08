#pragma once

#include <optional>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "render_info.h"
#include "utils.h"

inline void draw_scaled(const RenderInfo& ctx, SpriteId sprite_id, const SDL2pp::Rect& src, const SDL2pp::Rect& dst, bool flip = false, std::optional<SDL_Color> tint = std::nullopt) {
    SDL2pp::Texture& texture = ctx.assets.get_texture(sprite_id);

    if (tint) {
        texture.SetColorMod(tint->r, tint->g, tint->b);
    }

    ctx.r.Copy(texture, src, dst, 0.0, SDL2pp::NullOpt, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

    if (tint) {
        texture.SetColorMod(255, 255, 255);
    }
}

// Estira el recorte del sprite a `dst` (ignora sus offsets): iconos de UI, hotbar, etc.
inline void draw_sprite_in(const RenderInfo& ctx, const Sprite& s, const SDL2pp::Rect& dst, bool flip = false, std::optional<SDL_Color> tint = std::nullopt) {
    draw_scaled(ctx, s.sprite_id, s.src(), dst, flip, tint);
}

inline void draw_region(const RenderInfo& ctx, SpriteId sprite_id, const SDL2pp::Rect& src, const SDL2pp::Point& origin, int dx = 0, int dy = 0, bool flip = false, std::optional<SDL_Color> tint = std::nullopt) {
    const int z = ctx.cam.get_zoom();
    const SDL2pp::Rect dst(origin.x + dx * z, origin.y + dy * z, src.w * z, src.h * z);
    draw_scaled(ctx, sprite_id, src, dst, flip, tint);
}

// Dibuja el sprite usando su recorte y offsets; `origin` es el punto ancla en pantalla.
inline void draw_sprite(const RenderInfo& ctx, const Sprite& s, const SDL2pp::Point& origin, bool flip = false, std::optional<SDL_Color> tint = std::nullopt) {
    draw_region(ctx, s.sprite_id, s.src(), origin, s.dx(flip), s.offset_y, flip, tint);
}

inline void draw_texture_box(const RenderInfo& ctx, SpriteId sprite_id, const SDL2pp::Rect& src, const SDL2pp::Rect& dst, int corner, int scale) {
    const int c = corner * scale;
    const int sx[4] = {0, corner, src.w - corner, src.w};
    const int sy[4] = {0, corner, src.h - corner, src.h};
    const int dx[4] = {0, c, dst.w - c, dst.w};
    const int dy[4] = {0, c, dst.h - c, dst.h};

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            const SDL2pp::Rect s(src.x + sx[col], src.y + sy[row], sx[col + 1] - sx[col], sy[row + 1] - sy[row]);
            const SDL2pp::Rect d(dst.x + dx[col], dst.y + dy[row], dx[col + 1] - dx[col], dy[row + 1] - dy[row]);
            draw_scaled(ctx, sprite_id, s, d);
        }
    }
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

   
inline void draw_scaled_rotated(const RenderInfo& ctx, SpriteId sprite_id, const SDL2pp::Rect& src, const SDL2pp::Rect& dst, double angle, const SDL2pp::Point& pivot_px, bool flip = false, std::optional<SDL_Color> tint = std::nullopt) {
    SDL2pp::Texture& texture = ctx.assets.get_texture(sprite_id);
    const int z = dst.w / src.w;

    const int px = flip ? src.w - pivot_px.x : pivot_px.x;
    const SDL2pp::Point center(px * z, pivot_px.y * z);

    if (tint) {
        texture.SetColorMod(tint->r, tint->g, tint->b);
    }

    ctx.r.Copy(texture, src, dst, flip ? -angle : angle, center, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

    if (tint) {
        texture.SetColorMod(255, 255, 255);
    }
}

inline void draw_region_rotated(const RenderInfo& ctx, SpriteId sprite_id, const SDL2pp::Rect& src, const SDL2pp::Point& origin, int dx, int dy, double angle, const SDL2pp::Point& pivot_px, bool flip = false, std::optional<SDL_Color> tint = std::nullopt) {
    const int z = ctx.cam.get_zoom();
    const SDL2pp::Rect dst(origin.x + dx * z, origin.y + dy * z, src.w * z, src.h * z);
    draw_scaled_rotated(ctx, sprite_id, src, dst, angle, pivot_px, flip, tint);
}

inline void draw_sprite_rotated(const RenderInfo& ctx, const Sprite& s, const SDL2pp::Point& origin, double angle, const SDL2pp::Point& pivot_px, bool flip = false, std::optional<SDL_Color> tint = std::nullopt) {
    draw_region_rotated(ctx, s.sprite_id, s.src(), origin, s.dx(flip), s.offset_y, angle, pivot_px, flip, tint);
}
