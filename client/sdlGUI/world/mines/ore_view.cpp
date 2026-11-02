#include "ore_view.h"

#include "../../draw_region.h"
#include "../../render_info.h"

OreView::OreView(const MineralData& data): entityId(data.id), type(data.type), state(data.state == MineralState::Broken ? ResourceState::Broken : ResourceState::Intact) {
    update(data);
}

void OreView::update(const MineralData& data) {
    pos = data.pos;
    type = data.type;
    state = data.state == MineralState::Broken ? ResourceState::Broken : ResourceState::Intact;
}

bool OreView::is_hit(int world_x, int world_y) const {
    if (state == ResourceState::Broken) {
        return false;
    }
    return world_x >= pos.getX() && world_x < pos.getX() + TILE_TO_PIXEL &&
           world_y >= pos.getY() && world_y < pos.getY() + TILE_TO_PIXEL;
}

std::optional<float> OreView::depth(const RenderInfo& ctx) const {
    const int radius = TILE_TO_PIXEL * ctx.cam.get_zoom();

    if (!ctx.cam.is_visible(pos.getX(), pos.getY(), radius)) {
        return std::nullopt;
    }

    return static_cast<float>(pos.getY());
}

void OreView::draw(const RenderInfo& ctx) {
    const Sprite& sprite = ctx.assets.get_sprite(type, state);
    const SDL2pp::Point origin = ctx.cam.world_to_screen(pos.getX(), pos.getY());
    draw_region(ctx, sprite.sprite_id, sprite_rect(sprite), origin);
}