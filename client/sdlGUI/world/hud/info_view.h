#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <set>
#include <string>

#include <SDL2pp/SDL2pp.hh>

#include "../../../state.h"
#include "ui_box.h"

#include "protocol_types.h"
#include "game_time.h"

class InfoView {
private:
    static constexpr int SPRITE_SCALE = 3;
    static constexpr int RIGHT_MARGIN = 8;
    static constexpr int TOP_MARGIN = 8;

    static constexpr int KNOB_X = 22, KNOB_Y = 20;
    static constexpr int ARROW_PIVOT_X = 3, ARROW_PIVOT_Y = 16;

    static constexpr int PANEL_X = 27, PANEL_W = 39;
    static constexpr int TIME_PANEL_Y = 27, TIME_PANEL_H = 10;

    static constexpr int GOLD_SLOTS = 8;
    static constexpr int SLOTS_X = 16, SLOTS_W = 49, SLOTS_Y = 48, SLOTS_H = 8;

    static constexpr int ALERT_ICON_BOX = 44;
    static constexpr int ALERT_GAP = 6;
    static constexpr int ALERT_TOP_GAP = 12;
    static constexpr double ALERT_BOB_PHASE = 0.8;

    static constexpr TextStyle TEXT{.size = 1, .color = {107, 33, 0, 255}};
    static constexpr TextStyle ALERT{.size = 4, .color = {220, 40, 30, 255}, .shadow = pixel_font::SHADOW_COLOR, .shadow_offset = 1};

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

    void draw_gold(const RenderInfo& ctx, const UiBox& digits) const {
        const SDL2pp::Rect& r = digits.get_rect();
        const std::string text = std::to_string(std::min<uint32_t>(current_gold, 99999999u));
        const int first = GOLD_SLOTS - static_cast<int>(text.size());
        for (std::size_t i = 0; i < text.size(); ++i) {
            const int slot = first + static_cast<int>(i);
            UiBox(ctx, SDL2pp::Rect(r.x + slot * r.w / GOLD_SLOTS, r.y, r.w / GOLD_SLOTS, r.h), digits.get_unit()).text(std::string(1, text[i]), TEXT);
        }
    }

    void draw_alerts(const RenderInfo& ctx, const UiBox& screen, int top) const {
        int i = 0;
        for (const ItemId item : items_to_alert) {
            const int bob = bob_offset(ctx, BOB_AMPLITUDE, BOB_SPEED, i * ALERT_BOB_PHASE);
            const UiBox icon = screen.place(ALERT_ICON_BOX, ALERT_ICON_BOX, Anchor::TopRight,
                                            RIGHT_MARGIN + i * (ALERT_ICON_BOX + ALERT_GAP), top + bob);
            icon.fit(item);
            icon.text("!", ALERT, Anchor::TopRight, 0, -pixel_font::height(ALERT.size) / 2);
            ++i;
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
        const UiBox screen = UiBox::screen(ctx);
        const UiBox sign = screen.sprite(UiSpriteId::InfoSing, SPRITE_SCALE, Anchor::TopRight, RIGHT_MARGIN, TOP_MARGIN);

        const Sprite& arrow = ctx.assets.get_sprite(UiSpriteId::InfoSingArrow);
        sign.sub(KNOB_X - ARROW_PIVOT_X, KNOB_Y - ARROW_PIVOT_Y, arrow.get_width(), arrow.get_height()).blit(arrow, {.angle = hand_angle(), .pivot = SDL2pp::Point(ARROW_PIVOT_X, ARROW_PIVOT_Y)});
        sign.sub(PANEL_X, TIME_PANEL_Y, PANEL_W, TIME_PANEL_H).text(format_time(current_time), TEXT);
        draw_gold(ctx, sign.sub(SLOTS_X, SLOTS_Y, SLOTS_W, SLOTS_H));
        draw_alerts(ctx, screen, TOP_MARGIN + sign.get_rect().h / screen.get_unit() + ALERT_TOP_GAP);
    }
};