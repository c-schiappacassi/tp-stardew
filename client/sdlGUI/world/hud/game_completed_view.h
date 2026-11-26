#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "constants.h"
#include "ui_box.h"
#include "dancing_junimo_view.h"

class GameCompletedView {
private:
    inline static const std::string TITLE    = "Congratulations!";
    inline static const std::string SUBTITLE = "You completed the game!";
    inline static const std::string HINT     = "Press enter to exit";

    static constexpr SDL_Color TITLE_COLOR{255, 213, 74, 255};
    static constexpr SDL_Color TEXT_COLOR{255, 255, 255, 255};
    static constexpr SDL_Color SHADOW_COLOR{86, 22, 12, 255};
    static constexpr TextStyle HINT_STYLE{.size = 3, .color = TEXT_COLOR, .shadow = SHADOW_COLOR};

    static constexpr int FADE_STEP = 8;
    static constexpr double APPEAR_STEP_MS = 50;
    static constexpr double HINT_DELAY_MS = 1500;
    static constexpr double HINT_BLINK_MS = 700;

    static constexpr double TITLE_BOB_PHASE = 0.55;
    static constexpr int TEXT_CENTER_PERCENT = 42;

    static constexpr std::array<SDL_Color, 10> PALETTE{{
        {255,  90,  90, 255},
        {255, 160,  60, 255},
        {255, 230,  70, 255},
        {150, 255,  80, 255},
        { 80, 220, 110, 255},
        { 70, 230, 230, 255},
        { 90, 140, 255, 255},
        {170, 100, 255, 255},
        {255,  90, 220, 255},
        {255, 150, 190, 255},
    }};

    static constexpr int JUNIMO_COUNT = 40;

    struct Region { double x0, x1, y0, y1; };
    static bool inside(const Region& r, double x, double y) { return x >= r.x0 && x <= r.x1 && y >= r.y0 && y <= r.y1; }
    static constexpr Region TEXT_ZONE{0.14, 0.86, 0.31, 0.62};
    static constexpr Region HINT_ZONE{0.28, 0.72, 0.90, 1.00};

    int alpha = 0;
    long long content_ticks = 0;
    std::vector<DancingJunimoView> junimos;

    static double fract(double v) { return v - std::floor(v); }

    static constexpr std::array<JunimoState, 3> DANCES{JunimoState::DanceSlow, JunimoState::Dance, JunimoState::DanceFast};

   
    void build_junimos() {
        constexpr double A1 = 0.7548776662466927;
        constexpr double A2 = 0.5698402909980532;

        junimos.reserve(JUNIMO_COUNT);
        for (int n = 1; static_cast<int>(junimos.size()) < JUNIMO_COUNT && n < 20 * JUNIMO_COUNT; ++n) {
            const double x = 0.05 + 0.90 * fract(0.5 + n * A1);
            const double y = 0.14 + 0.82 * fract(0.5 + n * A2);
            if (inside(TEXT_ZONE, x, y) || inside(HINT_ZONE, x, y)) {
                continue;
            }
            const int i = static_cast<int>(junimos.size());
            junimos.emplace_back(
                x, y,
                PALETTE[(static_cast<std::size_t>(i) * 3) % PALETTE.size()],
                y < 0.40 ? -1 : (y > 0.70 ? 1 : 0),
                DANCES[static_cast<std::size_t>(i) % DANCES.size()],
                static_cast<double>((i * 7) % JUNIMO_COUNT) * APPEAR_STEP_MS,
                i % 2 == 0);
        }
        std::sort(junimos.begin(), junimos.end(), [](const DancingJunimoView& a, const DancingJunimoView& b) { return a.depth() < b.depth(); });
    }

    double content_ms() const { return static_cast<double>(content_ticks) * static_cast<double>(TICK_MS); }

    static int fit_px(const std::string& text, int max_w, int max_h) {
        return std::max(1, std::min(max_w / std::max(1, PixelFont::width(text, 1)), max_h / PixelFont::height(1)));
    }

    void draw_sky(const RenderInfo& ctx, const UiBox& screen) const {
        const SDL2pp::Point size = ctx.assets.get_texture(SpriteId::DaySky).GetSize();
        screen.cover(Sprite::from(SpriteId::DaySky, SDL2pp::Rect(0, 0, size.x, size.y)), {.alpha = static_cast<Uint8>(alpha)});
    }

    static void draw_title(const RenderInfo& ctx, const SDL2pp::Rect& screen) {
        const int w = screen.w, h = screen.h;
        const int title_px = fit_px(TITLE, w * 62 / 100, h * 16 / 100);
        const int sub_px = std::min(title_px, fit_px(SUBTITLE, w * 50 / 100, h * 8 / 100));
        const int block_h = PixelFont::height(title_px) + title_px * 3 + PixelFont::height(sub_px);
        const UiBox block(ctx, SDL2pp::Rect(0, h * TEXT_CENTER_PERCENT / 100 - block_h / 2, w, block_h));

        const TextStyle title{.size = title_px, .color = TITLE_COLOR, .shadow = SHADOW_COLOR};
        const int left = (w - PixelFont::width(TITLE, title_px)) / 2;
        for (std::size_t i = 0; i < TITLE.size(); ++i) {  // cada letra rebota con su propio desfase
            const int x = left + static_cast<int>(i) * (pixel_font::GLYPH_W + pixel_font::GLYPH_GAP) * title_px;
            const int bob = bob_offset(ctx, title_px * 6 / 5, BOB_SPEED, static_cast<double>(i) * TITLE_BOB_PHASE);
            block.text(std::string(1, TITLE[i]), title, Anchor::TopLeft, x, bob);
        }
        block.text(SUBTITLE, {.size = sub_px, .color = TEXT_COLOR, .shadow = SHADOW_COLOR}, Anchor::Bottom);
    }

public:
    GameCompletedView() { build_junimos(); }

    bool visible() const { return alpha > 0; }

    bool accepts_input() const { return alpha == 255 && content_ms() >= HINT_DELAY_MS; }

    void update(bool active) {
        alpha = active ? std::min(255, alpha + FADE_STEP) : std::max(0, alpha - FADE_STEP);
        content_ticks = (alpha == 255) ? content_ticks + 1 : 0;
    }

    void draw(const RenderInfo& ctx) {
        const UiBox screen = UiBox::screen(ctx);
        draw_sky(ctx, screen);
        if (alpha < 255) return;

        const double t = content_ms();
        for (DancingJunimoView& junimo : junimos) junimo.draw(ctx, screen, t);
        draw_title(ctx, screen.get_rect());
        if (accepts_input() && static_cast<long long>(t / HINT_BLINK_MS) % 2 == 0) {
            screen.text(HINT, HINT_STYLE, Anchor::Bottom, 0, 3 * HINT_STYLE.size);
        }
    }
};