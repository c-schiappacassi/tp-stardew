#include "level_view.h"

#include <algorithm>
#include <cstdint>

void LevelView::update(const  LevelData& level) {
    sync_views(ores, level.minerals, [](const MineralData& d) { return d.id; });
}

void LevelView::collect_entities(const RenderInfo& ctx, std::vector<Drawable>& out,  uint64_t& tie) {
    collect_from(ctx, ores, out, tie);
}