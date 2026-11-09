#include "hotbar_view.h"

#include <algorithm>
#include <string>

#include "../../draw_region.h"
#include "pixel_font.h"

namespace {
    constexpr int REFERENCE_HEIGHT = 720;
    constexpr int BAR_CORNER = 16;
    constexpr int SLOT_PX = 64;
    constexpr int BOTTOM_MARGIN = 8;
    constexpr int ICON_BOX = 48;

    constexpr int INDEX_MARGIN = 6;
    constexpr int INDEX_PX = 2;
    constexpr int QUANTITY_MARGIN = 4;
    constexpr int QUANTITY_PX = 3;

    struct Layout {
        int u;
        SDL2pp::Rect bar;
        int slots;

        SDL2pp::Rect slot(int i) const {
            const int size = SLOT_PX * u;
            return SDL2pp::Rect(bar.x + BAR_CORNER * u + i * size, bar.y + BAR_CORNER * u, size, size);
        }
    };

    Layout make_layout(const SDL2pp::Point& screen, int slots) {
        const int u = std::max(1, screen.y / REFERENCE_HEIGHT);
        const int pad = BAR_CORNER * u;
        const int w = slots * SLOT_PX * u + 2 * pad;
        const int h = SLOT_PX * u + 2 * pad;
        return Layout{u, SDL2pp::Rect((screen.x - w) / 2, screen.y - h - BOTTOM_MARGIN * u, w, h), slots};
    }

    SDL2pp::Rect icon_rect(const Sprite& icon, const SDL2pp::Rect& slot_rect, int u) {
        const int box = ICON_BOX * u;
        const int longest = std::max(icon.width, icon.height);
        const int w = std::max(1, icon.width * box / longest);
        const int h = std::max(1, icon.height * box / longest);
        return SDL2pp::Rect(slot_rect.x + (slot_rect.w - w) / 2, slot_rect.y + (slot_rect.h - h) / 2, w, h);
    }
}

void HotbarView::update(const PlayerDataLocal& local) {
    items = local.items;
    current_slot = local.current_slot;
    // this.cap = local.cap
}

void HotbarView::draw_stack(const RenderInfo& ctx, const Stack& stack, const SDL2pp::Rect& slot_rect, int u) {
    if (const Sprite* icon = ctx.assets.find_sprite(stack.item_id)) {
        draw_sprite_in(ctx, *icon, icon_rect(*icon, slot_rect, u));
    }

    if (stack.quantity > 1) {
        const int px = QUANTITY_PX * u;
        const std::string text = std::to_string(stack.quantity);
        const int x = slot_rect.x + slot_rect.w - QUANTITY_MARGIN * u - pixel_font::width(text, px);
        const int y = slot_rect.y + slot_rect.h - QUANTITY_MARGIN * u - pixel_font::height(px);
        pixel_font::draw_shadowed(ctx, text, x, y, px);
    }
}

void HotbarView::draw_slot(const RenderInfo& ctx, uint8_t slot, const SDL2pp::Rect& slot_rect, int u) {
    draw_sprite_in(ctx, ctx.assets.get_sprite(UiSpriteId::HotbarSlot), slot_rect);
    draw_sprite_in(ctx, ctx.assets.get_sprite(UiSpriteId::HotbarSlotOutline), slot_rect);

    if (const auto it = items.find(slot); it != items.end()) {
        draw_stack(ctx, it->second, slot_rect, u);
    }

    if (slot == current_slot) {
        draw_sprite_in(ctx, ctx.assets.get_sprite(UiSpriteId::HotbarSlotSelected), slot_rect);
    }

    pixel_font::draw_shadowed(ctx, std::to_string(slot + 1), slot_rect.x + INDEX_MARGIN * u, slot_rect.y + INDEX_MARGIN * u, INDEX_PX * u);
}

void HotbarView::draw(const RenderInfo& ctx) {
    const Layout layout = make_layout(ctx.r.GetOutputSize(), int(this->cap));

    const Sprite& bar = ctx.assets.get_sprite(UiSpriteId::HotbarBar);
    draw_texture_box(ctx, bar.sprite_id, bar.src(), layout.bar, BAR_CORNER, layout.u);

    for (int i = 0; i < layout.slots; ++i) {
        draw_slot(ctx, static_cast<uint8_t>(i), layout.slot(i), layout.u);
    }
}
