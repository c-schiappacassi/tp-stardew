#include "slot_view.h"

#include <string>

#include "../../draw_region.h"
#include "pixel_font.h"

namespace {
    constexpr int ICON_BOX = 48;

    constexpr int INDEX_MARGIN = 6;
    constexpr int INDEX_PX = 2;
    constexpr int QUANTITY_MARGIN = 4;
    constexpr int QUANTITY_PX = 3;
    constexpr int QUALITY_SCALE = 2;

    void draw_stack(const RenderInfo& ctx, const Stack& stack, const SDL2pp::Rect& slot_rect, int u) {
        draw_item_icon(ctx, stack.item_id, slot_rect, ICON_BOX * u);

        if (stack.quantity > 1) {
            draw_text_in(ctx, std::to_string(stack.quantity), slot_rect, QUANTITY_PX * u, HAlign::Right, VAlign::Bottom,
                         pixel_font::TEXT_COLOR, QUANTITY_MARGIN * u, QUANTITY_MARGIN * u,
                         pixel_font::SHADOW_COLOR, QUANTITY_PX * u);
        }

        if (stack.qual > Quality::Base) {
            const Sprite& star = ctx.assets.get_sprite(stack.qual);
            const int scale = QUALITY_SCALE * u;
            const SDL2pp::Rect icon_area = centered_rect(slot_rect, ICON_BOX * u, ICON_BOX * u);
            const SDL2pp::Rect dst = aligned_rect(icon_area, star.width * scale, star.height * scale, HAlign::Right, VAlign::Top, 0, 0);
            draw_sprite_in(ctx, star, dst);
        }
    }
}

void SlotView::draw(const RenderInfo& ctx, const SDL2pp::Rect& rect, int u) const {
    draw_ui(ctx, UiSpriteId::HotbarSlot, rect);
    draw_ui(ctx, UiSpriteId::HotbarSlotOutline, rect);

    if (stack) {
        draw_stack(ctx, *stack, rect, u);
    }

    if (selected) {
        draw_ui(ctx, UiSpriteId::HotbarSlotSelected, rect);
    }

    draw_text_in(ctx, label, rect, INDEX_PX * u, HAlign::Left, VAlign::Top,
                 pixel_font::TEXT_COLOR, INDEX_MARGIN * u, INDEX_MARGIN * u,
                 pixel_font::SHADOW_COLOR, INDEX_PX * u);
}
