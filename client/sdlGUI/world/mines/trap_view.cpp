#include "trap_view.h"

#include "../../draw_helpers.h"
#include "../../render_info.h"
#include "constants.h"

TrapView::TrapView(const TrapData& data) {
    trap_trigger.add<TrapSound>(sfx);
    update(data);
}

void TrapView::update(const TrapData& data) {
    sfx.clear();
    pos = data.pos;
    type = data.type;
    state = data.state;
    trap_trigger.observe(data.state, data.type, data.pos);

    if (data.dir != Direction::None) {
        facing = data.dir;
    }
}

std::optional<float> TrapView::depth(const RenderInfo& ctx) const {
    return depth_if_visible(ctx, pos,ctx.assets.get_height(type), ctx.assets.get_depth_offset(type));
}

void TrapView::draw(const RenderInfo& ctx) {
    sfx.flush(ctx);
    clip_player.draw_at(ctx, ClipId{ctx.assets.get_sprite_id(type), Action{state}, ctx.assets.get_clip_dir(type, facing)}, pos);
}