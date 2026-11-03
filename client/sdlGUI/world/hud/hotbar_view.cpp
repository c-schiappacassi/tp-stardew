#include "hotbar_view.h"

#include <algorithm>
#include <cstdint>
#include <string>

#include "../../draw_region.h"

#include "constants.h"

namespace {
    const SDL2pp::Rect BAR_SRC(0, 256, 60, 60);
    const SDL2pp::Rect SLOT_SRC(64, 128, 64, 64);
    const SDL2pp::Rect SLOT_SRC_OUTLINE(128, 128, 64, 64);
    const SDL2pp::Rect SELECTED_SRC(0, 896, 64, 64);
    constexpr int BAR_CORNER = 16;
    constexpr int SLOT_PX = 64;
    constexpr int ICON_PX = 16;
    constexpr int ICON_SCALE = 4;
    constexpr int BOTTOM_MARGIN = 8;

    constexpr int GLYPH_W = 3;
    constexpr int GLYPH_H = 5;
    constexpr int GLYPH_GAP = 1;
    constexpr uint8_t DIGITS[10][GLYPH_H] = {
        {0b111, 0b101, 0b101, 0b101, 0b111}, {0b010, 0b110, 0b010, 0b010, 0b111},
        {0b111, 0b001, 0b111, 0b100, 0b111}, {0b111, 0b001, 0b111, 0b001, 0b111},
        {0b101, 0b101, 0b111, 0b001, 0b001}, {0b111, 0b100, 0b111, 0b001, 0b111},
        {0b111, 0b100, 0b111, 0b101, 0b111}, {0b111, 0b001, 0b001, 0b001, 0b001},
        {0b111, 0b101, 0b111, 0b101, 0b111}, {0b111, 0b101, 0b111, 0b001, 0b111},
    };

    constexpr SDL_Color TEXT_COLOR{255, 255, 255, 255};
    constexpr SDL_Color SHADOW_COLOR{60, 30, 10, 255};

    void draw_digits(const RenderInfo& ctx, const std::string& text, int x, int y, int px, const SDL_Color& c) {
        ctx.r.SetDrawColor(c.r, c.g, c.b, c.a);
        for (const char ch: text) {
            const auto& glyph = DIGITS[ch - '0'];
            for (int row = 0; row < GLYPH_H; ++row) {
                for (int col = 0; col < GLYPH_W; ++col) {
                    if (glyph[row] & (0b100 >> col)) {
                        ctx.r.FillRect(SDL2pp::Rect(x + col * px, y + row * px, px, px));
                    }
                }
            }
            x += (GLYPH_W + GLYPH_GAP) * px;
        }
    }

    int text_width(const std::string& text, int px) {
        return (static_cast<int>(text.size()) * (GLYPH_W + GLYPH_GAP) - GLYPH_GAP) * px;
    }

    void draw_number(const RenderInfo& ctx, int value, int x, int y, int px) {
        const std::string text = std::to_string(value);
        draw_digits(ctx, text, x + px, y + px, px, SHADOW_COLOR);
        draw_digits(ctx, text, x, y, px, TEXT_COLOR);
    }
}

void HotbarView::update(const PlayerDataLocal& local) {
    items = local.items;
    current_slot = local.current_slot;
}

void HotbarView::draw_stack(const RenderInfo& ctx, const Stack& stack, const SDL2pp::Point& pos, int u) {
    const int icon = ICON_PX * ICON_SCALE * u;

    if (const Sprite* sprite = ctx.assets.find_sprite(stack.item_id)) {
        draw_scaled(ctx, sprite->sprite_id, sprite_rect(*sprite), SDL2pp::Rect(pos.x, pos.y, icon, icon));
    }

    if (stack.quantity > 1) {
        const int px = 3 * u;
        const std::string text = std::to_string(stack.quantity);
        const int x = pos.x + SLOT_PX * u - 4 * u - text_width(text, px);
        const int y = pos.y + SLOT_PX * u - 4 * u - GLYPH_H * px;
        draw_number(ctx, stack.quantity, x, y, px);
    }
}

void HotbarView::draw_slot(const RenderInfo& ctx, uint8_t slot, const SDL2pp::Point& pos, int u) {
    const int size = SLOT_PX * u;
    draw_scaled(ctx, SpriteId::MenuTiles, SLOT_SRC, SDL2pp::Rect(pos.x, pos.y, size, size));
    draw_scaled(ctx, SpriteId::MenuTiles, SLOT_SRC_OUTLINE, SDL2pp::Rect(pos.x, pos.y, size, size));

    const auto it = items.find(slot);
    if (it != items.end()) {
        draw_stack(ctx, it->second, pos, u);
    }

    if (slot == current_slot) {
        draw_scaled(ctx, SpriteId::MenuTiles, SELECTED_SRC, SDL2pp::Rect(pos.x, pos.y, size, size));
    }

    draw_number(ctx, slot + 1, pos.x + 6 * u, pos.y + 6 * u, 2 * u);
}

void HotbarView::draw(const RenderInfo& ctx) {
    const SDL2pp::Point out = ctx.r.GetOutputSize();
    const int u = std::max(1, out.y / 720);

    const int pad = BAR_CORNER * u;
    const int w = this->capacity * SLOT_PX * u + 2 * pad;
    const int h = SLOT_PX * u + 2 * pad;
    const SDL2pp::Rect bar((out.x - w) / 2, out.y - h - BOTTOM_MARGIN * u, w, h);

    draw_texture_box(ctx, SpriteId::MenuTiles, BAR_SRC, bar, BAR_CORNER, u);

    for (int i = 0; i < this->capacity; ++i) {
        const SDL2pp::Point pos(bar.x + pad + i * SLOT_PX * u, bar.y + pad);
        draw_slot(ctx, static_cast<uint8_t>(i), pos, u);
    }
}