#include "projectile_view.h"

#include "../../draw_region.h"
#include "../../render_info.h"

ProjectileView::ProjectileView(const ProjectileData& data) {
    update(data);
}

void ProjectileView::update(const ProjectileData& data) {
    pos = data.pos;
    type = data.type;
}

std::optional<float> ProjectileView::depth(const RenderInfo& ctx) const {
    return depth_if_visible(ctx, pos, dimentions::FLAME_SIZE );
}

void ProjectileView::draw(const RenderInfo& ctx) {
    draw_clip_at(ctx, clip_player, ClipId{SpriteId::Flames, Action{type}, Direction::None}, pos);
}
