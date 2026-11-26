#include "ore_view.h"

#include "../../draw_helpers.h"
#include "../../render_info.h"

OreView::OreView(const MineralData& data): type(data.type), state(data.state == MineralState::Broken ? ResourceState::Broken : ResourceState::Intact) {
    health_trigger.add<OreHitSound>(sfx);
    update(data);
}

void OreView::update(const MineralData& data) {
    sfx.clear();
    pos = data.pos;
    type = data.type;
    state = data.state == MineralState::Broken ? ResourceState::Broken : ResourceState::Intact;
    health_trigger.observe(data.health, data.pos);
}

std::optional<float> OreView::depth(const RenderInfo& ctx) const {
    return depth_if_visible(ctx, pos, TILE_TO_PIXEL);
}

void OreView::draw(const RenderInfo& ctx) {
    sfx.flush(ctx);
    ctx.assets.get_sprite(type, state).draw_at(ctx, pos);
}