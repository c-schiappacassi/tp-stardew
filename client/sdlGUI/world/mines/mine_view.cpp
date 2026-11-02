#include "mine_view.h"

void MinesView::update(const MineData& data) {
    current_level = data.current_level;
    auto [it, _] = levels.try_emplace(current_level, current_level);
    it->second.update(data.current_level_data);
}

void MinesView::draw_background(const RenderInfo& ctx) {
    const auto it = levels.find(current_level);
    if (it != levels.end()) {
        it->second.draw_background(ctx);
    }
}

void MinesView::collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) {
    const auto it = levels.find(current_level);
    if (it != levels.end()) {
        it->second.collect_drawables(ctx, out);
    }
}

std::optional<ClickTarget> MinesView::pick_target(int world_x, int world_y) const {
    const auto it = levels.find(current_level);
    return it == levels.end() ? std::nullopt : it->second.pick_target(world_x, world_y);
}