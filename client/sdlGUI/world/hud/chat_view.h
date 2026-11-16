#pragma once
#include <string>
#include <algorithm>
#include <SDL2/SDL.h>

#include "../../render_info.h"
#include "../../draw_region.h"
#include "pixel_font.h"

namespace {
    constexpr int REFERENCE_HEIGHT = 720;
    constexpr int SPRITE_SCALE = 3;
    constexpr int BOX_CORNER = 3;
    constexpr int BOX_W = 200;
    constexpr int BOX_H = 14;
    constexpr int PAD_X = 6;
    constexpr int LEFT_MARGIN = 8;
    constexpr int BOTTOM_RESERVED = 112;
    constexpr int TEXT_PX = 3;
    constexpr Uint32 BLINK_MS = 500;
}

class ChatView {
public:
    void draw(const RenderInfo& ctx, const std::string& text) const{
        const SDL2pp::Point screen = ctx.r.GetOutputSize();
        const int u = std::max(1, screen.y / REFERENCE_HEIGHT);
        const int s = SPRITE_SCALE * u;

        const int w = std::min(BOX_W * s, screen.x - 2 * LEFT_MARGIN * u);
        const int h = BOX_H * s;
        const SDL2pp::Rect box(LEFT_MARGIN * u, screen.y - BOTTOM_RESERVED * u - h, w, h);

        const Sprite& frame = ctx.assets.get_sprite(UiSpriteId::ChatBox);
        draw_texture_box(ctx, frame.sprite_id, frame.src(), box, BOX_CORNER, s);

        const int px = TEXT_PX * u;
        const int advance = (pixel_font::GLYPH_W + pixel_font::GLYPH_GAP) * px;
        const int text_x = box.x + PAD_X * s;
        const int text_y = box.y + (box.h - pixel_font::height(px)) / 2;

        const int inner_w = box.w - 2 * PAD_X * s;
        const std::size_t max_chars = static_cast<std::size_t>(std::max(1, inner_w / advance - 1));
        const std::string shown = text.size() > max_chars ? text.substr(text.size() - max_chars) : text;

        pixel_font::draw_shadowed(ctx, shown, text_x, text_y, px);

        if ((SDL_GetTicks() / BLINK_MS) % 2 == 0) {
            const int caret_x = text_x + static_cast<int>(shown.size()) * advance;
            ctx.r.SetDrawColor(255, 255, 255, 255);
            ctx.r.FillRect(SDL2pp::Rect(caret_x, text_y, px, pixel_font::height(px)));
        }
    }
};
