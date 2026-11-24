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
#include "../../draw_region.h"
#include "../../render_info.h"
#include "dancing_junimo_view.h"

class GameCompletedView {
private:
    inline static const std::string TITLE    = "Congratulations!";
    inline static const std::string SUBTITLE = "You completed the game!";
    inline static const std::string HINT     = "Press enter to exit";

    static constexpr SDL_Color TITLE_COLOR{255, 213, 74, 255};
    static constexpr SDL_Color TEXT_COLOR{255, 255, 255, 255};
    static constexpr SDL_Color SHADOW_COLOR{86, 22, 12, 255};

    static constexpr int FADE_STEP = 8;
    static constexpr double APPEAR_STEP_MS = 50;
    static constexpr double HINT_DELAY_MS = 1500;
    static constexpr double HINT_BLINK_MS = 700;

    static constexpr double TITLE_BOB_PHASE = 0.55;
    static constexpr int HINT_SCALE = 3;
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

    struct Region {
        double x0, x1, y0, y1;
        bool contains(double x, double y) const { return x >= x0 && x <= x1 && y >= y0 && y <= y1; }
    };
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
            if (TEXT_ZONE.contains(x, y) || HINT_ZONE.contains(x, y)) {
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
        const int by_w = max_w / std::max(1, PixelFont::width(text, 1));
        const int by_h = max_h / PixelFont::height(1);
        return std::max(1, std::min(by_w, by_h));
    }

    void draw_sky(const RenderInfo& ctx, const UiFrame& frame) const {
        SDL2pp::Texture& tex = ctx.assets.get_texture(SpriteId::DaySky);
        const SDL2pp::Point size = tex.GetSize();
        const SDL2pp::Point shown = fit_size(frame.size.x, frame.size.y, size.x, size.y);

        tex.SetBlendMode(SDL_BLENDMODE_BLEND);
        tex.SetAlphaMod(static_cast<Uint8>(alpha));
        draw_scaled(ctx, SpriteId::DaySky, centered_rect(SDL2pp::Rect(0, 0, size.x, size.y), shown.x, shown.y), frame.rect());
        tex.SetAlphaMod(255);
    }

    void draw_junimos(const RenderInfo& ctx, const UiFrame& frame, double t) {
        for (DancingJunimoView& junimo: junimos) {
            junimo.draw(ctx, frame, t);
        }
    }

    static void draw_title(const RenderInfo& ctx, const UiFrame& frame) {
        const int title_px = fit_px(TITLE, frame.size.x * 62 / 100, frame.size.y * 16 / 100);
        const int sub_px = std::min(title_px, fit_px(SUBTITLE, frame.size.x * 50 / 100, frame.size.y * 8 / 100));
        const int block_h = PixelFont::height(title_px) + title_px * 3 + PixelFont::height(sub_px);
        const SDL2pp::Rect block = rect_centered_on(SDL2pp::Point(frame.size.x / 2, frame.size.y * TEXT_CENTER_PERCENT / 100), frame.size.x, block_h);
        const SDL2pp::Rect title = text_rect_in(block, TITLE, title_px, HAlign::Center, VAlign::Top);

        draw_text_bobbing(ctx, TITLE, title.x, title.y, title_px, TITLE_COLOR, SHADOW_COLOR, std::max(1, title_px / 2), title_px * 6 / 5, TITLE_BOB_PHASE);
        draw_text_in(ctx, SUBTITLE, block, sub_px, HAlign::Center, VAlign::Bottom, TEXT_COLOR, 0, 0, SHADOW_COLOR, std::max(1, sub_px / 2));
    }

    static void draw_hint(const RenderInfo& ctx, const UiFrame& frame, double t) {
        if ((static_cast<long long>(t / HINT_BLINK_MS) % 2) != 0) {
            return;
        }
        const int px = HINT_SCALE * frame.u;
        draw_text_in(ctx, HINT, frame.rect(), px, HAlign::Center, VAlign::Bottom, TEXT_COLOR, 0, 3 * px, SHADOW_COLOR, std::max(1, px / 2));
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
        const UiFrame frame = ui_frame(ctx);
        draw_sky(ctx, frame);

        if (alpha < 255) {
            return;
        }
        const double t = content_ms();
        draw_junimos(ctx, frame, t);
        draw_title(ctx, frame);
        if (accepts_input()) {
            draw_hint(ctx, frame, t);
        }
    }
};