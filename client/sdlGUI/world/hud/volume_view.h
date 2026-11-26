#pragma once
#include <array>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../../draw_region.h"
#include "../../render_info.h"
#include "pixel_font.h"

class VolumeView {
public:
    struct Layout {
        SDL2pp::Rect box;
        std::vector<SDL2pp::Rect> buttons;
        int u = 1;
    };

private:
    inline static const std::array<std::string, 2> LABELS{"MUSIC", "SFX"};

    static constexpr int CORNER = 16;
    static constexpr int PAD = 32;
    static constexpr int BTN = 40;
    static constexpr int GAP = 12;
    static constexpr int SEGMENTS = 10;
    static constexpr int SEG_W = 12;
    static constexpr int SEG_GAP = 4;
    static constexpr int TEXT = 4;

    static constexpr SDL_Color DARK{86, 22, 12, 255};
    static constexpr SDL_Color TAN{222, 170, 105, 255};
    static constexpr SDL_Color TAN_HOVER{244, 206, 140, 255};
    static constexpr SDL_Color ON{62, 130, 50, 255};
    static constexpr SDL_Color OFF{176, 128, 78, 255};

    static void draw_button(const RenderInfo& ctx, const SDL2pp::Rect& r, const std::string& glyph, bool hover, int u) {
        const int b = 2 * u;
        fill_rect(ctx, r, DARK);
        fill_rect(ctx, SDL2pp::Rect(r.x + b, r.y + b, r.w - 2 * b, r.h - 2 * b), hover ? TAN_HOVER : TAN);
        draw_text_in(ctx, glyph, r, TEXT * u, HAlign::Center, VAlign::Center, DARK);
    }

    static void draw_bar(const RenderInfo& ctx, const SDL2pp::Rect& r, int lit, int u) {
        fill_rect(ctx, r, DARK);
        for (int i = 0; i < SEGMENTS; ++i) {
            const SDL2pp::Rect seg(r.x + (SEG_GAP + i * (SEG_W + SEG_GAP)) * u, r.y + SEG_GAP * u,
                                   SEG_W * u, r.h - 2 * SEG_GAP * u);
            fill_rect(ctx, seg, i < lit ? ON : OFF);
        }
    }

public:
    static Layout layout(const RenderInfo& ctx) {
        const UiFrame f = ui_frame(ctx);
        const int u = f.u;
        const int btn = BTN * u, gap = GAP * u;
        const int label_w = pixel_font::width(LABELS[0], TEXT * u);
        const int bar_w = (SEGMENTS * (SEG_W + SEG_GAP) + SEG_GAP) * u;

        const int w = label_w + btn + bar_w + btn + 3 * gap + 2 * PAD * u;
        const int h = 2 * btn + gap + 2 * PAD * u;
        const SDL2pp::Rect box = centered_rect(f.size, w, h);

        Layout l{box, {}, u};
        const int minus_x = box.x + PAD * u + label_w + gap;
        const int plus_x = minus_x + btn + gap + bar_w + gap;
        for (int row = 0; row < 2; ++row) {
            const int y = box.y + PAD * u + row * (btn + gap);
            l.buttons.emplace_back(minus_x, y, btn, btn);
            l.buttons.emplace_back(plus_x, y, btn, btn);
        }
        return l;
    }

    void draw(const RenderInfo& ctx, const Layout& l, const std::array<int, 2>& levels, int max_level,
              const std::vector<bool>& hovered) const {
        dim_screen(ctx, 110);
        draw_ui_box(ctx, UiSpriteId::DialogBox, l.box, CORNER, l.u);

        const int u = l.u, gap = GAP * u;
        for (std::size_t row = 0; row < 2; ++row) {
            const SDL2pp::Rect& minus = l.buttons[2 * row];
            const SDL2pp::Rect& plus = l.buttons[2 * row + 1];

            const int label_x = l.box.x + PAD * u;
            draw_text_in(ctx, LABELS[row], SDL2pp::Rect(label_x, minus.y, minus.x - gap - label_x, minus.h),
                         TEXT * u, HAlign::Left, VAlign::Center, DARK);

            const SDL2pp::Rect bar(minus.x + minus.w + gap, minus.y, plus.x - gap - (minus.x + minus.w + gap), minus.h);
            draw_button(ctx, minus, "-", hovered[2 * row], u);
            draw_bar(ctx, bar, levels[row] * SEGMENTS / max_level, u);
            draw_button(ctx, plus, "+", hovered[2 * row + 1], u);
        }
    }
};