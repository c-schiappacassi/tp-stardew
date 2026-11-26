#include "chest_inventory_view.h"

#include <algorithm>
#include <cstddef>

void ChestInventoryView::update(const PlayerDataLocal& local) {
    if (!local.openChest) {
        open_id.reset();
        slot_views.clear();
        return;
    }

    const OpenChestData& chest = *local.openChest;
    open_id = chest.id;
    slot_views.resize(chest.capacity);
    for (std::size_t i = 0; i < slot_views.size(); ++i) {
        const auto it = chest.items.find(static_cast<uint8_t>(i));
        slot_views[i].set_stack(it == chest.items.end() ? std::nullopt : std::optional<Stack>{it->second});
    }
}

std::optional<Stack> ChestInventoryView::stack_at(uint8_t slot) const {
    return slot < slot_views.size() ? slot_views[slot].get_stack() : std::nullopt;
}

void ChestInventoryView::draw(const RenderInfo& ctx, Hitboxes& hits) {
    if (!open_id || slot_views.empty()) return;
    using namespace slot_layout;

    const int count = static_cast<int>(slot_views.size());
    const int cols = std::min(COLUMNS, count);
    const int rows = (count + cols - 1) / cols;

    const UiBox panel = UiBox::screen(ctx).place(cols * SLOT_PX + 2 * PANEL_CORNER, rows * SLOT_PX + 2 * PANEL_CORNER);
    panel.frame(UiSpriteId::HotbarBar, PANEL_CORNER);

    for (int i = 0; i < count; ++i) {
        const UiBox slot = panel.sub(PANEL_CORNER + (i % cols) * SLOT_PX, PANEL_CORNER + (i / cols) * SLOT_PX, SLOT_PX, SLOT_PX);
        SlotView& view = slot_views[static_cast<std::size_t>(i)];
        view.set_selected(hits.add(slot.get_rect()));
        view.draw(slot);
    }
}
