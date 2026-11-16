#pragma once
#include <algorithm>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../../draw_region.h"
#include "../../render_info.h"
#include "pixel_font.h"

class DialogView {
public:
    struct Spec {
        std::vector<std::string> lines;
        std::vector<UiSpriteId> buttons;
        int button_top_gap = 12;
    };

    struct Layout {
        SDL2pp::Rect box;
        std::vector<SDL2pp::Rect> buttons;
        int u = 1;
        int text_y = 0;
    };

private:
    static constexpr int REFERENCE_HEIGHT = 720;
    static constexpr int BOX_CORNER = 16;
    static constexpr int PAD_X = 20;
    static constexpr int PAD_Y = 16;
    static constexpr int TEXT_SIZE = 4;
    static constexpr int LINE_GAP = 10;
    static constexpr int BUTTON_SIZE = 64;
    static constexpr int BUTTON_GAP = 24;
    static constexpr int MIN_TEXT_W = 220;
    static constexpr Uint8 DIM_ALPHA = 110;

    static constexpr SDL_Color TEXT_COLOR{86, 22, 12, 255};
    static constexpr SDL_Color SHADOW_COLOR{206, 156, 95, 255};

    static void draw_button(const RenderInfo& ctx, UiSpriteId id, const SDL2pp::Rect& r, bool hover) {
        const int grow = hover ? r.w / 20 : 0;
        const SDL2pp::Rect dst(r.x - grow, r.y - grow, r.w + 2 * grow, r.h + 2 * grow);
        draw_sprite_in(ctx, ctx.assets.get_sprite(id), dst);
    }

public:
    static Layout layout(const RenderInfo& ctx, const Spec& spec) {
        const SDL2pp::Point screen = ctx.r.GetOutputSize();
        const int u = std::max(1, screen.y / REFERENCE_HEIGHT);
        const int px = TEXT_SIZE * u;
        const int btn = BUTTON_SIZE * u;
        const int gap = BUTTON_GAP * u;

        int text_w = MIN_TEXT_W * u;
        for (const auto& line : spec.lines) {
            text_w = std::max(text_w, pixel_font::width(line, px));
        }
        const int line_count = static_cast<int>(spec.lines.size());
        const int text_h = line_count > 0
            ? line_count * pixel_font::height(px) + (line_count - 1) * LINE_GAP * u
            : 0;

        const int btn_count = static_cast<int>(spec.buttons.size());
        const int buttons_w = btn_count > 0 ? btn_count * btn + (btn_count - 1) * gap : 0;
        const int buttons_h = btn_count > 0 ? btn : 0;
        const int top_gap = (line_count > 0 && btn_count > 0) ? spec.button_top_gap * u : 0;

        const int inner_w = std::max(text_w, buttons_w);
        const int inner_h = text_h + top_gap + buttons_h;

        const int w = inner_w + 2 * (BOX_CORNER + PAD_X) * u;
        const int h = inner_h + 2 * (BOX_CORNER + PAD_Y) * u;
        const SDL2pp::Rect box((screen.x - w) / 2, (screen.y - h) / 2, w, h);

        std::vector<SDL2pp::Rect> btn_rects;
        btn_rects.reserve(btn_count);
        if (btn_count > 0) {
            int bx = box.x + (w - buttons_w) / 2;
            const int by = box.y + h - (BOX_CORNER + PAD_Y) * u - btn;
            for (int i = 0; i < btn_count; ++i) {
                btn_rects.emplace_back(bx, by, btn, btn);
                bx += btn + gap;
            }
        }

        return Layout{box, std::move(btn_rects), u, box.y + (BOX_CORNER + PAD_Y) * u};
    }

    void draw(const RenderInfo& ctx, const Spec& spec, const Layout& l, const std::vector<bool>& hovered) const {
        const SDL2pp::Point screen = ctx.r.GetOutputSize();

        ctx.r.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
        ctx.r.SetDrawColor(0, 0, 0, DIM_ALPHA);
        ctx.r.FillRect(SDL2pp::Rect(0, 0, screen.x, screen.y));
        ctx.r.SetDrawBlendMode(SDL_BLENDMODE_NONE);

        const Sprite& frame = ctx.assets.get_sprite(UiSpriteId::DialogBox);
        draw_texture_box(ctx, frame.sprite_id, frame.src(), l.box, BOX_CORNER, l.u);

        const int px = TEXT_SIZE * l.u;
        const int shadow = std::max(1, px / 2);
        int y = l.text_y;
        for (const auto& line : spec.lines) {
            const int x = l.box.x + (l.box.w - pixel_font::width(line, px)) / 2;
            pixel_font::draw(ctx, line, x + shadow, y + shadow, px, SHADOW_COLOR);
            pixel_font::draw(ctx, line, x, y, px, TEXT_COLOR);
            y += pixel_font::height(px) + LINE_GAP * l.u;
        }

        for (std::size_t i = 0; i < spec.buttons.size() && i < l.buttons.size(); ++i) {
            const bool is_hover = i < hovered.size() && hovered[i];
            draw_button(ctx, spec.buttons[i], l.buttons[i], is_hover);
        }
    }
};