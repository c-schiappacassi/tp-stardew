#include "projectile_view.h"

#include "../../draw_helpers.h"
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
    clip_player.draw_at(ctx, ClipId{SpriteId::Flames, Action{type}, Direction::None}, pos);
}
