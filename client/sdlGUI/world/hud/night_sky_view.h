#pragma once

#include <algorithm>

#include "ui_box.h"

class NightSkyView {
private: 
    static constexpr int FADE_STEP = 8;
    int alpha = 0; 

public:
    bool visible() const { return alpha > 0; }

    void update(const bool& active) {
        alpha = active ? std::min(255, alpha + FADE_STEP) : std::max(0, alpha - FADE_STEP);
    }

    void draw(const RenderInfo& ctx) const {
        UiBox::screen(ctx).cover(ctx.assets.get_sprite(UiSpriteId::NightSky), {.alpha = static_cast<Uint8>(alpha)});
    }
};
