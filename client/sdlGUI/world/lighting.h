#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "../draw_region.h"
#include "../render_info.h"
#include "game_time.h"

namespace lighting {
    inline constexpr uint16_t DUSK_START = 18 * 60;
    inline constexpr uint16_t NIGHT_FULL = 21 * 60;

    inline constexpr SDL_Color DAYLIGHT{255, 255, 255, 255};
    inline constexpr SDL_Color NIGHT_OUTDOORS{120, 130, 190, 255};
    inline constexpr SDL_Color DUNGEON_DARK{115, 115, 135, 255};

    inline constexpr int       PLAYER_GLOW_RADIUS    = 56;
    inline constexpr int       PLAYER_GLOW_OFFSET_Y  = -14;
    inline constexpr SDL_Color PLAYER_GLOW_COLOR{255, 190, 120, 255};
    inline constexpr double    PLAYER_GLOW_INTENSITY = 0.6;

    inline uint8_t lerp_u8(uint8_t a, uint8_t b, double t) {
        return static_cast<uint8_t>(a + (b - a) * t + 0.5);
    }

    inline SDL_Color lerp(const SDL_Color& a, const SDL_Color& b, double t) {
        return SDL_Color{lerp_u8(a.r, b.r, t), lerp_u8(a.g, b.g, t), lerp_u8(a.b, b.b, t), 255};
    }

    inline bool is_white(const SDL_Color& c) {
        return c.r == 255 && c.g == 255 && c.b == 255;
    }

    inline double darkness(uint16_t minutes) {
        const unsigned t = minutes % game_time::MINUTES_PER_24_HOURS;
        if (t < game_time::START_MINUTES) return 1.0;
        if (t <= DUSK_START) return 0.0;
        if (t >= NIGHT_FULL) return 1.0;

        const double x = static_cast<double>(t - DUSK_START) / (NIGHT_FULL - DUSK_START);
        return x * x * (3.0 - 2.0 * x);
    }

    inline SDL_Color outdoor_ambient(const RenderInfo& ctx) {
        return lerp(DAYLIGHT, NIGHT_OUTDOORS, darkness(ctx.state.get_local_time()));
    }

    inline SDL_Color dungeon_ambient() {
        return DUNGEON_DARK;
    }

    struct Light {
        SDL2pp::Point center;
        int radius;
        SDL_Color color;
        double intensity;
    };
}

class LightMap {
private:
    std::optional<SDL2pp::Texture> target;
    SDL2pp::Point target_size{0, 0};

    static SDL_BlendMode additive_blend() {
        static const SDL_BlendMode mode = SDL_ComposeCustomBlendMode(
            SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD,
            SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD);
        return mode;
    }

    void ensure_target(SDL2pp::Renderer& r, const SDL2pp::Point& size) {
        if (target && target_size == size) return;
        target.emplace(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size.x, size.y);
        target->SetBlendMode(SDL_BLENDMODE_MOD);
        target_size = size;
    }

    static Uint8 scaled(Uint8 channel, double k) {
        return static_cast<Uint8>(std::clamp(channel * k, 0.0, 255.0) + 0.5);
    }

public:
    void draw(const RenderInfo& ctx, const SDL_Color& ambient, const std::vector<lighting::Light>& lights) {
        if (lighting::is_white(ambient)) return;

        const SDL2pp::Point screen = ctx.r.GetOutputSize();
        ensure_target(ctx.r, screen);

        ctx.r.SetTarget(*target);
        ctx.r.SetDrawBlendMode(SDL_BLENDMODE_NONE);
        ctx.r.SetDrawColor(ambient.r, ambient.g, ambient.b, 255);
        ctx.r.Clear();

        SDL2pp::Texture& glow = ctx.assets.get_texture(SpriteId::LightGlow);
        glow.SetBlendMode(additive_blend());
        for (const lighting::Light& l : lights) {
            ScopedTint tint(glow, SDL_Color{scaled(l.color.r, l.intensity), scaled(l.color.g, l.intensity), scaled(l.color.b, l.intensity), 255});
            ctx.r.Copy(glow, SDL2pp::NullOpt, rect_centered_on(l.center, 2 * l.radius, 2 * l.radius));
        }

        ctx.r.SetTarget();
        ctx.r.Copy(*target);
    }
};