#include "chest_inventory_view.h"

#include <algorithm>
#include <cstddef>

#include "../../draw_region.h"

void ChestInventoryView::close() {
    open_id.reset();
    selected.reset();
    slot_views.clear();
    slot_rects.clear();
}

void ChestInventoryView::apply_selection() {
    for (std::size_t i = 0; i < slot_views.size(); ++i) {
        slot_views[i].set_selected(selected.has_value() && *selected == i);
    }
}

void ChestInventoryView::update(const PlayerDataLocal& local) {
    if (!local.openChest) {
        close();
        return;
    }

    const OpenChestData& chest = *local.openChest;
    if (open_id != chest.id) {
        selected.reset();
    }
    open_id = chest.id;

    slot_views.resize(chest.capacity);
    for (std::size_t i = 0; i < slot_views.size(); ++i) {
        const auto it = chest.items.find(static_cast<uint8_t>(i));
        slot_views[i].set_stack(it == chest.items.end() ? std::nullopt : std::optional<Stack>{it->second});
    }
    apply_selection();
}

void ChestInventoryView::select(std::optional<uint8_t> slot) {
    selected = slot;
    apply_selection();
}

const std::vector<SDL2pp::Rect>& ChestInventoryView::rects() const { return slot_rects; }

std::optional<Stack> ChestInventoryView::stack_at(uint8_t slot) const { 
    return slot < slot_views.size() ? slot_views[slot].get_stack() : std::nullopt; 
}

void ChestInventoryView::draw(const RenderInfo& ctx) {
    if (!open_id || slot_views.empty()) {
        slot_rects.clear();
        return;
    }

    const UiFrame frame = ui_frame(ctx);
    const int u = frame.u;
    const int slot_size = slot_layout::SLOT_PX * u;
    const int pad = slot_layout::PANEL_CORNER * u;

    const int cols = std::min<int>(COLUMNS, static_cast<int>(slot_views.size()));
    const int rows = (static_cast<int>(slot_views.size()) + cols - 1) / cols;

    const SDL2pp::Rect panel = aligned_rect(frame.rect(), cols * slot_size + 2 * pad, rows * slot_size + 2 * pad, HAlign::Center, VAlign::Center);
    draw_ui_box(ctx, UiSpriteId::HotbarBar, panel, slot_layout::PANEL_CORNER, u);

    slot_rects.clear();
    slot_rects.reserve(slot_views.size());
    for (std::size_t i = 0; i < slot_views.size(); ++i) {
        const int col = static_cast<int>(i) % cols;
        const int row = static_cast<int>(i) / cols;
        const SDL2pp::Rect rect(panel.x + pad + col * slot_size, panel.y + pad + row * slot_size, slot_size, slot_size);
        slot_rects.push_back(rect);
        slot_views[i].draw(ctx, rect, u);
    }
}
