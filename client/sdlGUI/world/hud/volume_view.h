#pragma once
#include <array>
#include <string>

#include "../../ui/hitboxes.h"
#include "ui_box.h"

class VolumeView {
private:
    inline static const std::array<std::string, 2> LABELS{"MUSIC", "SFX"};

    static constexpr int CORNER = 16;
    static constexpr int PAD = 32;
    static constexpr int BTN = 40;
    static constexpr int GAP = 12;
    static constexpr int BORDER = 2;
    static constexpr int SEGMENTS = 10;
    static constexpr int SEG_W = 12;
    static constexpr int SEG_GAP = 4;
    static constexpr int BAR_W = SEGMENTS * (SEG_W + SEG_GAP) + SEG_GAP;

    static constexpr SDL_Color DIM{0, 0, 0, 110};
    static constexpr SDL_Color DARK{86, 22, 12, 255};
    static constexpr SDL_Color TAN{222, 170, 105, 255};
    static constexpr SDL_Color TAN_HOVER{244, 206, 140, 255};
    static constexpr SDL_Color ON{62, 130, 50, 255};
    static constexpr SDL_Color OFF{176, 128, 78, 255};
    static constexpr TextStyle TEXT{.size = 4, .color = DARK};

    static void button(const UiBox& b, const std::string& glyph, Hitboxes& hits) {
        const bool hover = hits.add(b.get_rect());
        b.fill(DARK);
        b.inset(BORDER, BORDER).fill(hover ? TAN_HOVER : TAN);
        b.text(glyph, TEXT);
    }

    static void bar(const UiBox& b, int lit) {
        b.fill(DARK);
        const UiBox segments = b.inset(SEG_GAP, SEG_GAP);
        for (int i = 0; i < SEGMENTS; ++i) {
            segments.sub(i * (SEG_W + SEG_GAP), 0, SEG_W, BTN - 2 * SEG_GAP).fill(i < lit ? ON : OFF);
        }
    }

public:
    static void draw(const RenderInfo& ctx, const std::array<int, 2>& levels, int max_level, Hitboxes& hits) {
        const UiBox screen = UiBox::screen(ctx);
        screen.fill(DIM);

        const int label_w = pixel_font::width(LABELS[0], TEXT.size);
        const int row_w = label_w + BTN + BAR_W + BTN + 3 * GAP;
        const UiBox box = screen.place(row_w + 2 * PAD, 2 * BTN + GAP + 2 * PAD);
        box.frame(UiSpriteId::DialogBox, CORNER);

        for (int r = 0; r < 2; ++r) {
            const UiBox row = box.sub(PAD, PAD + r * (BTN + GAP), row_w, BTN);
            row.sub(0, 0, label_w, BTN).text(LABELS[r], TEXT, Anchor::Left);
            const int minus_x = label_w + GAP, bar_x = minus_x + BTN + GAP, plus_x = bar_x + BAR_W + GAP;
            button(row.sub(minus_x, 0, BTN, BTN), "-", hits);
            bar(row.sub(bar_x, 0, BAR_W, BTN), levels[r] * SEGMENTS / max_level);
            button(row.sub(plus_x, 0, BTN, BTN), "+", hits);
        }
    }
};