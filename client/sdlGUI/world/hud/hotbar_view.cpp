#include "hotbar_view.h"

#include <algorithm>
#include <string>

#include "../../draw_region.h"

namespace {
    constexpr int BOTTOM_MARGIN = 8;

    struct Layout {
        int u;
        SDL2pp::Rect bar;
        int slots;

        SDL2pp::Rect slot(int i) const {
            const int size = slot_layout::SLOT_PX * u;
            return SDL2pp::Rect(bar.x + slot_layout::PANEL_CORNER * u + i * size, bar.y + slot_layout::PANEL_CORNER * u, size, size);
        }
    };

    Layout make_layout(const UiFrame& f, int slots) {
        const int pad = slot_layout::PANEL_CORNER * f.u;
        const int w = slots * slot_layout::SLOT_PX * f.u + 2 * pad;
        const int h = slot_layout::SLOT_PX * f.u + 2 * pad;
        return Layout{f.u, aligned_rect(f.rect(), w, h, HAlign::Center, VAlign::Bottom, 0, BOTTOM_MARGIN * f.u), slots};
    }
}

HotbarView::HotbarView(): slot_views(SLOTS) {
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
    const Layout layout = make_layout(ui_frame(ctx), static_cast<int>(slot_views.size()));
 
    draw_ui_box(ctx, UiSpriteId::HotbarBar, layout.bar, slot_layout::PANEL_CORNER, layout.u);
 
    for (int i = 0; i < layout.slots; ++i) {
        slot_views[static_cast<std::size_t>(i)].draw(ctx, layout.slot(i), layout.u);
    }
}

std::vector<SDL2pp::Rect> HotbarView::slot_rects(const RenderInfo& ctx) {
    const Layout layout = make_layout(ui_frame(ctx), SLOTS);
 
    std::vector<SDL2pp::Rect> rects;
    rects.reserve(SLOTS);
    for (int i = 0; i < layout.slots; ++i) {
        rects.push_back(layout.slot(i));
    }
    return rects;
}
