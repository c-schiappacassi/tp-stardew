#include "slot_view.h"

#include <string>

namespace {
    constexpr int ICON_BOX = 48;
    constexpr int INDEX_MARGIN = 6;
    constexpr int QUANTITY_MARGIN = 4;
    constexpr int QUALITY_SCALE = 2;

    constexpr TextStyle INDEX{.size = 2, .color = pixel_font::TEXT_COLOR, .shadow = pixel_font::SHADOW_COLOR, .shadow_offset = 2};
    constexpr TextStyle QUANTITY{.size = 3, .color = pixel_font::TEXT_COLOR, .shadow = pixel_font::SHADOW_COLOR, .shadow_offset = 3};
}

void SlotView::draw(const UiBox& slot) const {
    slot.blit(UiSpriteId::HotbarSlot);
    slot.blit(UiSpriteId::HotbarSlotOutline);

    if (stack) {
        const UiBox icon = slot.place(ICON_BOX, ICON_BOX);
        icon.fit(stack->item_id);
        if (stack->quantity > 1) {
            slot.text(std::to_string(stack->quantity), QUANTITY, Anchor::BottomRight, QUANTITY_MARGIN, QUANTITY_MARGIN);
        }
        if (stack->qual > Quality::Base) {
            icon.sprite(stack->qual, QUALITY_SCALE, Anchor::TopRight);
        }
    }

    if (selected) slot.blit(UiSpriteId::HotbarSlotSelected);

    slot.text(label, INDEX, Anchor::TopLeft, INDEX_MARGIN, INDEX_MARGIN);
}
