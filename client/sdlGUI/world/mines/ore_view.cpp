#include "ore_view.h"

#include "../../draw_region.h"
#include "../../render_info.h"

OreView::OreView(const MineralData& data): type(data.type), state(data.state == MineralState::Broken ? ResourceState::Broken : ResourceState::Intact) {
    update(data);
}

void OreView::update(const MineralData& data) {
    pos = data.pos;
    type = data.type;
    state = data.state == MineralState::Broken ? ResourceState::Broken : ResourceState::Intact;
}

std::optional<float> OreView::depth(const RenderInfo& ctx) const {
    return depth_if_visible(ctx, pos, TILE_TO_PIXEL);
}

void OreView::draw(const RenderInfo& ctx) {
    const Sprite& sprite = ctx.assets.get_sprite(type, state);
    draw_sprite_at(ctx, sprite, pos);
}