#include "level_view.h"

#include <algorithm>
#include <cstdint>

void LevelView::update(const LevelData& level) {
    sync_views(ores, level.minerals, [](const MineralData& d) { return d.id; });
    sync_views(enemies, level.combat.enemies, [](const EnemySnapshot& d) { return d.id; });
    sync_views(traps, level.combat.traps, [](const TrapData& d) { return d.id; });
    sync_views(projectiles, level.combat.projectiles, [](const ProjectileData& d) { return d.id; });
}

void LevelView::collect_entities(const RenderInfo& ctx, std::vector<Drawable>& out, uint64_t& tie) {
    collect_from(ctx, ores, out, tie);
    collect_from(ctx, enemies, out, tie);
    collect_from(ctx, traps, out, tie);
    collect_from(ctx, projectiles, out, tie);
}