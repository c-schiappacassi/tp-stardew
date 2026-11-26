#pragma once
#include <algorithm>
#include <string>
#include <vector>

#include "../../ui/hitboxes.h"
#include "ui_box.h"

class DialogView {
public:
    struct Spec {
        std::vector<std::string> lines;
        std::vector<UiSpriteId> buttons;
        int button_top_gap = 12;
    };

private:
    static constexpr int CORNER = 16;
    static constexpr int PAD_X = 20 + CORNER;
    static constexpr int PAD_Y = 16 + CORNER;
    static constexpr int LINE_GAP = 10;
    static constexpr int BUTTON = 64;
    static constexpr int BUTTON_GAP = 24;
    static constexpr int HOVER_GROW = 3;
    static constexpr int MIN_TEXT_W = 220;
    static constexpr SDL_Color DIM{0, 0, 0, 110};
    static constexpr TextStyle TEXT{.size = 4, .color = {86, 22, 12, 255}, .shadow = SDL_Color{206, 156, 95, 255}};

    static int stack(int n, int size, int gap) { return n > 0 ? n * (size + gap) - gap : 0; }

public:
    static void draw(const RenderInfo& ctx, const Spec& spec, Hitboxes& hits) {
        const UiBox screen = UiBox::screen(ctx);
        screen.fill(DIM);

        const int lines = static_cast<int>(spec.lines.size());
        const int buttons = static_cast<int>(spec.buttons.size());
        const int line_h = pixel_font::height(TEXT.size) + LINE_GAP;

        int inner_w = std::max(MIN_TEXT_W, stack(buttons, BUTTON, BUTTON_GAP));
        for (const auto& line : spec.lines) inner_w = std::max(inner_w, pixel_font::width(line, TEXT.size));
        const int top_gap = lines > 0 && buttons > 0 ? spec.button_top_gap : 0;
        const int inner_h = stack(lines, line_h - LINE_GAP, LINE_GAP) + top_gap + (buttons > 0 ? BUTTON : 0);

        const UiBox box = screen.place(inner_w + 2 * PAD_X, inner_h + 2 * PAD_Y);
        box.frame(UiSpriteId::DialogBox, CORNER);
        const UiBox inner = box.inset(PAD_X, PAD_Y);

        for (int i = 0; i < lines; ++i) {
            inner.sub(0, i * line_h, inner_w, 0).text(spec.lines[i], TEXT, Anchor::Top);
        }

        const UiBox row = inner.place(stack(buttons, BUTTON, BUTTON_GAP), BUTTON, Anchor::Bottom);
        for (int i = 0; i < buttons; ++i) {
            const UiBox button = row.sub(i * (BUTTON + BUTTON_GAP), 0, BUTTON, BUTTON);
            (hits.add(button.get_rect()) ? button.inset(-HOVER_GROW, -HOVER_GROW) : button).blit(spec.buttons[i]);
        }
    }
};