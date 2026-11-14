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
    const int radius = Dimentions::FLAME_CELL_H * ctx.cam.get_zoom();
    if (!ctx.cam.is_visible(pos.getX(), pos.getY(), radius)) {
        return std::nullopt;
    }

    return static_cast<float>(pos.getY());
}

void ProjectileView::draw(const RenderInfo& ctx) {
    const ClipId id{SpriteId::Flames, Action{type}, Direction::None};

    if (!current_clip || !(*current_clip == id)) {
        current_clip = id;
        anim.reset();
    }

    if (last_it >= 0 && ctx.it > last_it) {
        anim.update(static_cast<float>(ctx.it - last_it) * TICK_MS);
    }
    last_it = ctx.it;

    if (!ctx.assets.has_clip(id)) {
        return;
    }

    const Clip& clip = ctx.assets.get_clip(id);
    const AnimatedFrame& frame = anim.get_current_frame(clip);

    const SDL2pp::Point origin = ctx.cam.world_to_screen(pos.getX(), pos.getY());
    draw_sprite(ctx, frame.sprite, origin, frame.flip);
}
