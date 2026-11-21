#pragma once

#include "../../draw_region.h"
#include "../../render_info.h"

class NightSkyView {
private: 
    static constexpr int FADE_STEP = 8;
    int alpha = 0; 

public:
    bool visible() const { return alpha > 0; }

    void update(const bool& active) {
        alpha = active ? std::min(255, alpha + FADE_STEP) : std::max(0, alpha - FADE_STEP);
    }

    void draw(const RenderInfo& ctx) {
        const SDL2pp::Point screen = ctx.r.GetOutputSize();
        const Sprite& sky = ctx.assets.get_sprite(UiSpriteId::NightSky);

        const double k = std::max(static_cast<double>(screen.x) / sky.width, static_cast<double>(screen.y) / sky.height);
        const int w = static_cast<int>(std::ceil(sky.width * k));
        const int h = static_cast<int>(std::ceil(sky.height * k));

        SDL2pp::Texture& tex = ctx.assets.get_texture(sky.sprite_id);
        tex.SetBlendMode(SDL_BLENDMODE_BLEND);
        tex.SetAlphaMod(static_cast<Uint8>(alpha));
        draw_ui(ctx, UiSpriteId::NightSky, centered_rect(screen, w, h));
        tex.SetAlphaMod(255);
    }
};
