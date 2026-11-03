#include "level_view.h"

#include <algorithm>
#include <cstdint>

void LevelView::update(const  LevelData& level) {
    sync_views(ores, level.minerals, [](const MineralData& d) { return d.id; });
}

void LevelView::draw_background(const RenderInfo&) {
    // TODO: implementar background
}

void LevelView::collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) {
    uint64_t tie = static_cast<uint64_t>(1ull) << 32;
    collect_from(ctx, ores, out, tie);
}