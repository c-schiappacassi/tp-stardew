#pragma once

#include <optional>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "render_info.h"
#include "utils.h"

inline void draw_region(const RenderInfo& ctx, SpriteId sprite_id, const SDL2pp::Rect& src, const SDL2pp::Point& origin, int dx = 0, int dy = 0, bool flip = false, std::optional<SDL_Color> tint = std::nullopt) {
    const int z = ctx.cam.get_zoom();

    SDL2pp::Texture& texture = ctx.assets.get_texture(sprite_id);

    const SDL2pp::Rect dst(origin.x + dx * z, origin.y + dy * z, src.w * z, src.h * z);

    if (tint) {
        texture.SetColorMod(tint->r, tint->g, tint->b);
    }

    ctx.r.Copy(texture, src, dst, 0.0, SDL2pp::NullOpt, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

    if (tint) {
        texture.SetColorMod(255, 255, 255);
    }
}

inline SDL2pp::Rect sprite_rect(const Sprite& sprite) {
    return SDL2pp::Rect(
        sprite.col * sprite.width,
        sprite.row * sprite.height,
        sprite.width,
        sprite.height
    );
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

   