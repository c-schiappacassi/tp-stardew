#include "trap_view.h"

#include "../../draw_region.h"
#include "../../render_info.h"
#include "constants.h"

TrapView::TrapView(const TrapData& data) {
    update(data);
}

void TrapView::update(const TrapData& data) {
    pos = data.pos;
    type = data.type;
    state = data.state;

    if (data.dir != Direction::None) {
        facing = data.dir;
    }
}

std::optional<float> TrapView::depth(const RenderInfo& ctx) const {
    return depth_if_visible(ctx, pos,ctx.assets.get_height(type), ctx.assets.get_depth_offset(type));
}

void TrapView::draw(const RenderInfo& ctx) {
    draw_clip_at(ctx, clip_player,ClipId{ctx.assets.get_sprite_id(type), Action{state}, ctx.assets.get_clip_dir(type, facing)}, pos);
}