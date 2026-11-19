#include "hotbar_view.h"

#include <algorithm>
#include <string>

#include "../../draw_region.h"
#include "pixel_font.h"

namespace {
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

    Layout make_layout(const UiFrame& f, int slots) {
        const int pad = BAR_CORNER * f.u;
        const int w = slots * SLOT_PX * f.u + 2 * pad;
        const int h = SLOT_PX * f.u + 2 * pad;
        return Layout{f.u, aligned_rect(f.rect(), w, h, HAlign::Center, VAlign::Bottom, 0, BOTTOM_MARGIN * f.u), slots};
    }
}

void HotbarView::update(const PlayerDataLocal& local) {
    items = local.items;
    current_slot = local.current_slot;
    // this.cap = local.cap
}

void HotbarView::draw_stack(const RenderInfo& ctx, const Stack& stack, const SDL2pp::Rect& slot_rect, int u) {
    draw_item_icon(ctx, stack.item_id, slot_rect, ICON_BOX * u);

    if (stack.quantity > 1) {
        draw_text_in(ctx, std::to_string(stack.quantity), slot_rect, QUANTITY_PX * u, HAlign::Right, VAlign::Bottom,
                     pixel_font::TEXT_COLOR, QUANTITY_MARGIN * u, QUANTITY_MARGIN * u,
                     pixel_font::SHADOW_COLOR, QUANTITY_PX * u);
    }
}

void HotbarView::draw_slot( const RenderInfo& ctx, uint8_t slot, const SDL2pp::Rect& rect, int u) {
    draw_ui(ctx, UiSpriteId::HotbarSlot, rect);
    draw_ui(ctx, UiSpriteId::HotbarSlotOutline, rect);

    if (const auto it = items.find(slot); it != items.end()) {
        draw_stack(ctx, it->second, rect, u);
    }

    if (slot == current_slot) {
        draw_ui(ctx, UiSpriteId::HotbarSlotSelected, rect);
    }

    draw_text_in( ctx, std::to_string(slot + 1), rect, INDEX_PX * u, HAlign::Left, VAlign::Top, 
        pixel_font::TEXT_COLOR, INDEX_MARGIN * u, INDEX_MARGIN * u, pixel_font::SHADOW_COLOR, INDEX_PX * u
    );
}

void HotbarView::draw(const RenderInfo& ctx) {
    const Layout layout = make_layout(ui_frame(ctx), int(this->cap));

    draw_ui_box(ctx, UiSpriteId::HotbarBar, layout.bar, BAR_CORNER, layout.u);

    for (int i = 0; i < layout.slots; ++i) {
        draw_slot(ctx, static_cast<uint8_t>(i), layout.slot(i), layout.u);
    }
}