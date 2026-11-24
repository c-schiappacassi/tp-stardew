#include "bar_view.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "../../draw_region.h"
#include "pixel_font.h"

namespace {
    constexpr int SPRITE_SCALE = 3;
    constexpr int FILL_INSET_X = 3;
    constexpr int FILL_INSET_TOP = 13;
    constexpr int FILL_INSET_BOTTOM = 2;
    constexpr int RIGHT_MARGIN = 8;
    constexpr int BOTTOM_MARGIN = 8;
    constexpr int BAR_GAP = 4;
    constexpr int LABEL_PX = 3;
    constexpr int LABEL_GAP = 4;

    SDL_Color fill_color(float ratio) {
        const float r = (ratio <= 0.5f ? 1.0f : 1.0f - 2.0f * (ratio - 0.5f)) * 220.0f;
        const float g = std::sqrt(ratio) * 150.0f;
        return SDL_Color{static_cast<Uint8>(r), static_cast<Uint8>(g), 0, 255};
    }
}

void BarView::draw(const RenderInfo& ctx) const {
    if (max_value == 0) {
        return;
    }
    if (hide_when_full && value >= max_value) {
        return;
    }

    const UiFrame f = ui_frame(ctx);
    const int s = SPRITE_SCALE * f.u;

    const Sprite& frame = ctx.assets.get_sprite(frame_sprite);
    const int w = frame.width * s;
    const int h = frame.height * s;
    const SDL2pp::Rect bar = aligned_rect(f.rect(), w, h, HAlign::Right, VAlign::Bottom,
                                          RIGHT_MARGIN * f.u + order * (w + BAR_GAP * f.u), BOTTOM_MARGIN * f.u);

    draw_sprite_in(ctx, frame, bar);

    const SDL2pp::Rect inner(bar.x + FILL_INSET_X * s, bar.y + FILL_INSET_TOP * s, bar.w - 2 * FILL_INSET_X * s, bar.h - (FILL_INSET_TOP + FILL_INSET_BOTTOM) * s);

    const float ratio = std::clamp(static_cast<float>(value) / static_cast<float>(max_value), 0.0f, 1.0f);
    int fill_h = static_cast<int>(std::round(static_cast<float>(inner.h) * ratio));
    if (value > 0) {
        fill_h = std::max(fill_h, s);
    }
    if (fill_h > 0) {
        const SDL2pp::Rect fill(inner.x, inner.y + inner.h - fill_h, inner.w, fill_h);
        fill_rect(ctx, fill, fill_color(ratio));
    }
}