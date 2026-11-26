#include "hotbar_view.h"

#include <string>

namespace {
    constexpr int BOTTOM_MARGIN = 8;

    UiBox bar_box(const RenderInfo& ctx, int count) {
        using namespace slot_layout;
        return UiBox::screen(ctx).place(count * SLOT_PX + 2 * PANEL_CORNER, SLOT_PX + 2 * PANEL_CORNER, Anchor::Bottom, 0, BOTTOM_MARGIN);
    }

    UiBox slot_box(const UiBox& bar, int i) {
        return bar.sub(slot_layout::PANEL_CORNER + i * slot_layout::SLOT_PX, slot_layout::PANEL_CORNER, slot_layout::SLOT_PX, slot_layout::SLOT_PX);
    }
}

HotbarView::HotbarView(): slot_views(COUNT) {
    for (std::size_t i = 0; i < slot_views.size(); ++i) {
        slot_views[i].set_label(std::to_string(i + 1));
    }
}

void HotbarView::update(const PlayerDataLocal& local) {
    for (std::size_t i = 0; i < slot_views.size(); ++i) {
        const auto it = local.items.find(static_cast<uint8_t>(i));
        slot_views[i].set_stack(it == local.items.end() ? std::nullopt : std::optional<Stack>{it->second});
        slot_views[i].set_selected(i == local.current_slot);
    }
}

void HotbarView::draw(const RenderInfo& ctx) {
    const UiBox bar = bar_box(ctx, COUNT);
    bar.frame(UiSpriteId::HotbarBar, slot_layout::PANEL_CORNER);
    for (int i = 0; i < COUNT; ++i) {
        slot_views[static_cast<std::size_t>(i)].draw(slot_box(bar, i));
    }
}

std::vector<SDL2pp::Rect> HotbarView::slot_rects(const RenderInfo& ctx) {
    const UiBox bar = bar_box(ctx, COUNT);
    std::vector<SDL2pp::Rect> rects;
    for (int i = 0; i < COUNT; ++i) {
        rects.push_back(slot_box(bar, i).get_rect());
    }
    return rects;
}
