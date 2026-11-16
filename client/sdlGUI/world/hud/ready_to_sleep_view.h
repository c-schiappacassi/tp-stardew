#pragma once
#include <algorithm>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../../render_info.h"
#include "../../draw_region.h"
#include "pixel_font.h"

class ReadyToSleepView {
public:
    struct Layout {
        SDL2pp::Rect box;
        SDL2pp::Rect ok;
        int u;
    };

private:
    static constexpr int REFERENCE_HEIGHT = 720;
    static constexpr int BOX_CORNER = 16;
    static constexpr int PAD_X = 20;
    static constexpr int PAD_Y = 16;
    static constexpr int TEXT_SIZE = 4;
    static constexpr int LINE_GAP = 10;
    static constexpr int OK_SIZE = 64;
    static constexpr int OK_GAP = 12;
    static constexpr int MIN_TEXT_W = 220;
    static constexpr Uint8 DIM_ALPHA = 110;

    static constexpr SDL_Color TEXT_COLOR{86, 22, 12, 255};
    static constexpr SDL_Color SHADOW_COLOR{206, 156, 95, 255};

    static const std::vector<std::string>& lines(bool waiting) {
        static const std::vector<std::string> ask{"The day is over.", "Ready to go to sleep?"};
        static const std::vector<std::string> wait{"Waiting for other players..."};
        return waiting ? wait : ask;
    }

public:
    static Layout layout(const RenderInfo& ctx, bool waiting) {
        const SDL2pp::Point screen = ctx.r.GetOutputSize();
        const int u = std::max(1, screen.y / REFERENCE_HEIGHT);
        const int px = TEXT_SIZE * u;

        const auto& text = lines(waiting);
        int text_w = MIN_TEXT_W * u;
        for (const std::string& line: text) {
            text_w = std::max(text_w, pixel_font::width(line, px));
        }
        const int count = static_cast<int>(text.size());
        const int text_h = count * pixel_font::height(px) + (count - 1) * LINE_GAP * u;

        const int ok = waiting ? 0 : OK_SIZE * u;
        const int gap = waiting ? 0 : OK_GAP * u;

        const int w = text_w + 2 * (BOX_CORNER + PAD_X) * u;
        const int h = text_h + gap + ok + 2 * (BOX_CORNER + PAD_Y) * u;

        const SDL2pp::Rect box((screen.x - w) / 2, (screen.y - h) / 2, w, h);
        const SDL2pp::Rect ok_rect(box.x + (w - ok) / 2, box.y + h - (BOX_CORNER + PAD_Y) * u - ok, ok, ok);
        return Layout{box, ok_rect, u};
    }

    void draw(const RenderInfo& ctx, const Layout& l, bool waiting, bool ok_hover) const {
        const SDL2pp::Point screen = ctx.r.GetOutputSize();

        ctx.r.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
        ctx.r.SetDrawColor(0, 0, 0, DIM_ALPHA);
        ctx.r.FillRect(SDL2pp::Rect(0, 0, screen.x, screen.y));
        ctx.r.SetDrawBlendMode(SDL_BLENDMODE_NONE);

        const Sprite& frame = ctx.assets.get_sprite(UiSpriteId::DialogBox);
        draw_texture_box(ctx, frame.sprite_id, frame.src(), l.box, BOX_CORNER, l.u);

        const int px = TEXT_SIZE * l.u;
        const int shadow = std::max(1, px / 2);
        int y = l.box.y + (BOX_CORNER + PAD_Y) * l.u;
        for (const std::string& line: lines(waiting)) {
            const int x = l.box.x + (l.box.w - pixel_font::width(line, px)) / 2;
            pixel_font::draw(ctx, line, x + shadow, y + shadow, px, SHADOW_COLOR);
            pixel_font::draw(ctx, line, x, y, px, TEXT_COLOR);
            y += pixel_font::height(px) + LINE_GAP * l.u;
        }

        if (waiting) {
            return;
        }

        const int grow = ok_hover ? l.ok.w / 20 : 0;
        const SDL2pp::Rect ok(l.ok.x - grow, l.ok.y - grow, l.ok.w + 2 * grow, l.ok.h + 2 * grow);
        draw_sprite_in(ctx, ctx.assets.get_sprite(UiSpriteId::OkButton), ok);
    }
};