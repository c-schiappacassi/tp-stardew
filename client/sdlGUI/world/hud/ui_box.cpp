#include "ui_box.h"

#include <algorithm>
#include <cmath>

SDL2pp::Rect anchored_rect(const SDL2pp::Rect& area, int w, int h, Anchor a, int mx, int my) {
    const int col = static_cast<int>(a) % 3;
    const int row = static_cast<int>(a) / 3;
    const int x = col == 0 ? area.x + mx : col == 2 ? area.x + area.w - mx - w : area.x + (area.w - w) / 2;
    const int y = row == 0 ? area.y + my : row == 2 ? area.y + area.h - my - h : area.y + (area.h - h) / 2;
    return SDL2pp::Rect(x, y, w, h);
}

UiBox UiBox::screen(const RenderInfo& ctx) {
    constexpr int REFERENCE_HEIGHT = 720;
    const SDL2pp::Point size = ctx.r.GetOutputSize();
    return UiBox(ctx, SDL2pp::Rect(0, 0, size.x, size.y), std::max(1, size.y / REFERENCE_HEIGHT));
}

UiBox UiBox::sub(int x, int y, int w, int h) const {
    return UiBox(ctx, SDL2pp::Rect(rect.x + x * unit, rect.y + y * unit, w * unit, h * unit), unit);
}

UiBox UiBox::inset(int dx, int dy) const {
    return UiBox(ctx, SDL2pp::Rect(rect.x + dx * unit, rect.y + dy * unit, rect.w - 2 * dx * unit, rect.h - 2 * dy * unit), unit);
}

UiBox UiBox::place(int w, int h, Anchor a, int mx, int my) const {
    return UiBox(ctx, anchored_rect(rect, w * unit, h * unit, a, mx * unit, my * unit), unit);
}

void UiBox::fill(const SDL_Color& c) const {
    if (c.a < 255) ctx.r.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    ctx.r.SetDrawColor(c.r, c.g, c.b, c.a);
    ctx.r.FillRect(rect);
    if (c.a < 255) ctx.r.SetDrawBlendMode(SDL_BLENDMODE_NONE);
}

UiBox UiBox::scaled_to(const Sprite& s, bool cover, const DrawOpts& opts) const {
    const UiBox box(ctx, scaled_rect(rect, s, cover), unit);
    s.blit(ctx, box.rect, opts);
    return box;
}