#pragma once

#include <optional>

#include <SDL2/SDL.h>

#include "health_trigger.h"
#include "../../render_info.h"

class HitFlash : public HealthListener {
    static constexpr long long DURATION_TICKS = 12;
    static constexpr int PEAK_GB = 70;

    bool pending = false;
    bool running = false;
    long long start_it = 0;

public:
    void on_damage(const HealthChange&) override { pending = true; }

    std::optional<SDL_Color> tint(const RenderInfo& ctx) {
        if (pending) {
            pending = false;
            running = true;
            start_it = ctx.it;
        }
        if (!running) return std::nullopt;

        const long long elapsed = ctx.it - start_it;
        if (elapsed >= DURATION_TICKS) {
            running = false;
            return std::nullopt;
        }

        const float t = static_cast<float>(elapsed) / static_cast<float>(DURATION_TICKS);
        const Uint8 gb = static_cast<Uint8>(PEAK_GB + (255 - PEAK_GB) * t);
        return SDL_Color{255, gb, gb, 255};
    }
};

inline std::optional<SDL_Color> combine_tint(const std::optional<SDL_Color>& a, const std::optional<SDL_Color>& b) {
    if (!a) return b;
    if (!b) return a;
    return SDL_Color{ static_cast<Uint8>(a->r * b->r / 255), static_cast<Uint8>(a->g * b->g / 255), static_cast<Uint8>(a->b * b->b / 255), 255};
}