#include "bar_view.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "../../draw_region.h"
#include "pixel_font.h"

namespace {
    constexpr int REFERENCE_HEIGHT = 720;
    constexpr int FRAME_CORNER = 16;
    constexpr int FRAME_THICKNESS = 12;
    constexpr int BAR_W = 44;
    constexpr int BAR_H = 144;
    constexpr int FILL_MARGIN = 2;
    constexpr int RIGHT_MARGIN = 8;
    constexpr int BOTTOM_MARGIN = 8;
    constexpr int BAR_GAP = 4;
    constexpr int LABEL_PX = 3;
    constexpr int LABEL_GAP = 4;

    constexpr SDL_Color EMPTY_COLOR{60, 30, 10, 255};

    SDL_Color fill_color(float ratio) {
        const float r = (ratio <= 0.5f ? 1.0f : 1.0f - 2.0f * (ratio - 0.5f)) * 220.0f;
        const float g = std::sqrt(ratio) * 150.0f;
        return SDL_Color{static_cast<Uint8>(r), static_cast<Uint8>(g), 0, 255};
    }

    void fill_rect(const RenderInfo& ctx, const SDL2pp::Rect& rect, const SDL_Color& c) {
        ctx.r.SetDrawColor(c.r, c.g, c.b, c.a);
        ctx.r.FillRect(rect);
    }
}

void BarView::draw(const RenderInfo& ctx) const {
    if (max_value == 0) {
        return;
    }
    if (hide_when_full && value >= max_value) {
        return;
    }

    const SDL2pp::Point screen = ctx.r.GetOutputSize();
    const int u = std::max(1, screen.y / REFERENCE_HEIGHT);

    const int w = BAR_W * u;
    const int h = BAR_H * u;
    const int x = screen.x - RIGHT_MARGIN * u - w - order * (w + BAR_GAP * u);
    const int y = screen.y - BOTTOM_MARGIN * u - h;
    const SDL2pp::Rect bar(x, y, w, h);

    const Sprite& frame = ctx.assets.get_sprite(UiSpriteId::HotbarBar);
    draw_texture_box(ctx, frame.sprite_id, frame.src(), bar, FRAME_CORNER, u);

    const int t = FRAME_THICKNESS * u;
    const SDL2pp::Rect inner(bar.x + t, bar.y + t, bar.w - 2 * t, bar.h - 2 * t);
    fill_rect(ctx, inner, EMPTY_COLOR);

    const float ratio = std::clamp(static_cast<float>(value) / static_cast<float>(max_value), 0.0f, 1.0f);
    const int m = FILL_MARGIN * u;
    const int max_fill_h = inner.h - 2 * m;
    int fill_h = static_cast<int>(std::round(static_cast<float>(max_fill_h) * ratio));
    if (value > 0) {
        fill_h = std::max(fill_h, u);
    }
    if (fill_h > 0) {
        const SDL2pp::Rect fill(inner.x + m, inner.y + inner.h - m - fill_h, inner.w - 2 * m, fill_h);
        fill_rect(ctx, fill, fill_color(ratio));
    }

    const std::string text(1, label);
    const int px = LABEL_PX * u;
    const int lx = bar.x + (bar.w - pixel_font::width(text, px)) / 2;
    const int ly = bar.y - LABEL_GAP * u - pixel_font::height(px);
    pixel_font::draw_shadowed(ctx, text, lx, ly, px);
}