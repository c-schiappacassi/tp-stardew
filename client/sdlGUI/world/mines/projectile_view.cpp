#include "projectile_view.h"

#include "../../draw_region.h"
#include "../../render_info.h"
#include "../../sprite_defs.h"
#include "constants.h"

ProjectileView::ProjectileView(const ProjectileData& data) {
    update(data);
}

void ProjectileView::update(const ProjectileData& data) {
    pos = data.pos;
    type = data.type;
}

std::optional<float> ProjectileView::depth(const RenderInfo& ctx) const {
    return depth_if_visible(ctx, pos, Dimentions::FLAME_CELL_H);
}

void ProjectileView::draw(const RenderInfo& ctx) {
    draw_clip_at(ctx, clip_player, ClipId{SpriteId::Flames, Action{type}, Direction::None}, pos);
}
