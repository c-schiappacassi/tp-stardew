#include "bar_view.h"

namespace {
    constexpr int SPRITE_SCALE = 3;
    constexpr int FILL_INSET_X = 3;
    constexpr int FILL_INSET_TOP = 13;
    constexpr int FILL_INSET_BOTTOM = 2;
    constexpr int RIGHT_MARGIN = 8;
    constexpr int BOTTOM_MARGIN = 8;
    constexpr int BAR_GAP = 4;

    SDL_Color fill_color(float ratio) {
        const float r = (ratio <= 0.5f ? 1.0f : 1.0f - 2.0f * (ratio - 0.5f)) * 220.0f;
        const float g = std::sqrt(ratio) * 150.0f;
        return SDL_Color{static_cast<Uint8>(r), static_cast<Uint8>(g), 0, 255};
    }
}

void BarView::draw(const RenderInfo& ctx) const {
    if (max_value == 0 || (hide_when_full && value >= max_value)) return;

    const Sprite& frame = ctx.assets.get_sprite(frame_sprite);
    const int stride = frame.get_width() * SPRITE_SCALE + BAR_GAP;
    const UiBox bar = UiBox::screen(ctx).sprite(frame, SPRITE_SCALE, Anchor::BottomRight, RIGHT_MARGIN + order * stride, BOTTOM_MARGIN);
    const UiBox inner = bar.sub(FILL_INSET_X, FILL_INSET_TOP, frame.get_width() - 2 * FILL_INSET_X, frame.get_height() - FILL_INSET_TOP - FILL_INSET_BOTTOM);
    const float ratio = std::clamp(static_cast<float>(value) / static_cast<float>(max_value), 0.0f, 1.0f);
    
    int fill_h = static_cast<int>(std::round(static_cast<float>(inner.get_rect().h) * ratio));
    if (value > 0) fill_h = std::max(fill_h, inner.get_unit());
    if (fill_h > 0) UiBox(ctx, anchored_rect(inner.get_rect(), inner.get_rect().w, fill_h, Anchor::Bottom)).fill(fill_color(ratio));
}