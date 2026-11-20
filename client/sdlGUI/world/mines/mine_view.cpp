#include "mine_view.h"

void MinesView::update(const MineData& data) {
    current_level = data.current_level;
    const auto it = levels.find(current_level);
    if (it != levels.end()) {
        it->second.update(data.current_level_data);
    }
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

SDL_Color MinesView::ambient_light(const RenderInfo&) { return lighting::dungeon_ambient(); }