#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <set>
#include <string>

#include <SDL2pp/SDL2pp.hh>

#include "../../../state.h"
#include "../../draw_region.h"
#include "../../render_info.h"
#include "pixel_font.h"

#include "protocol_types.h"
#include "game_time.h"

class InfoView {
private:
    static constexpr int REFERENCE_HEIGHT = 720;
    static constexpr int SPRITE_SCALE = 3;
    static constexpr int RIGHT_MARGIN = 8;
    static constexpr int TOP_MARGIN = 8;

    static constexpr int KNOB_X = 22, KNOB_Y = 20;
    static constexpr int ARROW_PIVOT_X = 3, ARROW_PIVOT_Y = 16;

    static constexpr int ICON_W = 12, ICON_H = 8;

    static constexpr int PANEL_X = 27, PANEL_W = 39;
    static constexpr int TIME_PANEL_Y = 27, TIME_PANEL_H = 10;

    static constexpr int GOLD_SLOTS = 8;
    static constexpr int SLOTS_X = 16, SLOTS_W = 49, SLOTS_Y = 48, SLOTS_H = 8;

    static constexpr int ALERT_ICON_BOX = 44;
    static constexpr int ALERT_BANG_PX = 4;
    static constexpr int ALERT_GAP = 6;
    static constexpr int ALERT_TOP_GAP = 12;

    static constexpr SDL_Color TEXT_COLOR{107, 33, 0, 255};
    static constexpr SDL_Color ALERT_RED{220, 40, 30, 255};

    uint32_t current_gold = 0;
    uint16_t current_time = game_time::START_MINUTES;
    std::set<ItemId> items_to_alert;

    static std::string format_time(uint16_t minutes) {
        const unsigned total = minutes % game_time::MINUTES_PER_24_HOURS;
        const unsigned h = total / 60;
        const unsigned m = (total % 60) / 10 * 10;
        const unsigned h12 = (h % 12 == 0) ? 12 : h % 12;
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%u:%02u %s", h12, m, h < 12 ? "am" : "pm");
        return buf;
    }

    double hand_angle() const {
        constexpr double START = game_time::START_MINUTES;
        constexpr double SPAN  = game_time::MINUTES_PER_24_HOURS - START;
        const double t = current_time;
        const double elapsed = (t < START) ? SPAN : std::clamp(t - START, 0.0, SPAN);
        return 180.0 + 180.0 * (elapsed / SPAN);
    }

    void draw_time(const RenderInfo& ctx, const SDL2pp::Point& sign, int s) const {
        const std::string text = format_time(current_time);
        const int px = s;
        const int w = pixel_font::width(text, px);
        const int x = sign.x + PANEL_X * s + (PANEL_W * s - w) / 2;
        const int y = sign.y + TIME_PANEL_Y * s + (TIME_PANEL_H * s - pixel_font::height(px)) / 2;
        pixel_font::draw(ctx, text, x, y, px, TEXT_COLOR);
    }

    void draw_gold(const RenderInfo& ctx, const SDL2pp::Point& sign, int s) const {
        const std::string text = std::to_string(std::min<uint32_t>(current_gold, 99999999u));
        const int px = s;
        const int y = sign.y + SLOTS_Y * s + (SLOTS_H * s - pixel_font::height(px)) / 2;
        const int digits = static_cast<int>(text.size());

        for (int i = 0; i < digits; ++i) {
            const int slot = GOLD_SLOTS - digits + i;
            const int slot_x = sign.x + SLOTS_X * s + slot * SLOTS_W * s / GOLD_SLOTS;
            const int slot_w = SLOTS_W * s / GOLD_SLOTS;
            const int x = slot_x + (slot_w - pixel_font::GLYPH_W * px) / 2;
            pixel_font::draw(ctx, std::string(1, text[i]), x, y, px, TEXT_COLOR);
        }
    }

    void draw_alerts(const RenderInfo& ctx, const SDL2pp::Point& screen, int u, int below_y) const {
        const int box_size = ALERT_ICON_BOX * u;
        const int step = box_size + ALERT_GAP * u;
        const int bob = static_cast<int>(std::lround(std::sin(static_cast<double>(ctx.it) * 0.15) * 2.0)) * u;

        int x = screen.x - RIGHT_MARGIN * u - box_size;
        const int y = below_y + ALERT_TOP_GAP * u + bob;

        for (const ItemId item : items_to_alert) {
            if (const Sprite* icon = ctx.assets.find_sprite(item)) {
                const int longest = std::max<int>(icon->width, icon->height);
                const int w = std::max(1, icon->width * box_size / longest);
                const int h = std::max(1, icon->height * box_size / longest);
                draw_sprite_in(ctx, *icon,
                               SDL2pp::Rect(x + (box_size - w) / 2, y + (box_size - h) / 2, w, h));
            }

            const int bpx = ALERT_BANG_PX * u;
            const int bx = x + box_size - pixel_font::width("!", bpx);
            const int by = y - pixel_font::height(bpx) / 2;
            pixel_font::draw(ctx, "!", bx + u, by + u, bpx, pixel_font::SHADOW_COLOR);
            pixel_font::draw(ctx, "!", bx, by, bpx, ALERT_RED);

            x -= step;
        }
    }

public:
    InfoView() = default;

    void update(const PlayerDataLocal& local, const uint16_t& time, const std::set<ItemId>& to_alert) {
        current_gold = local.gold;
        current_time = time;
        items_to_alert = to_alert;
    }

    void draw(const RenderInfo& ctx) const {
        const SDL2pp::Point screen = ctx.r.GetOutputSize();
        const int u = std::max(1, screen.y / REFERENCE_HEIGHT);
        const int s = SPRITE_SCALE * u;

        const Sprite& sign = ctx.assets.get_sprite(UiSpriteId::InfoSing);
        const Sprite& arrow = ctx.assets.get_sprite(UiSpriteId::InfoSingArrow);

        const SDL2pp::Point origin(screen.x - RIGHT_MARGIN * u - sign.width * s, TOP_MARGIN * u);
        draw_sprite_in(ctx, sign, SDL2pp::Rect(origin.x, origin.y, sign.width * s, sign.height * s));

        const SDL2pp::Rect arrow_dst(origin.x + (KNOB_X - ARROW_PIVOT_X) * s, origin.y + (KNOB_Y - ARROW_PIVOT_Y) * s, arrow.width * s, arrow.height * s);
        draw_scaled_rotated(ctx, arrow.sprite_id, arrow.src(), arrow_dst, hand_angle(),  SDL2pp::Point(ARROW_PIVOT_X, ARROW_PIVOT_Y));

        draw_time(ctx, origin, s);
        draw_gold(ctx, origin, s);

        draw_alerts(ctx, screen, u, origin.y + sign.height * s);
    }
};