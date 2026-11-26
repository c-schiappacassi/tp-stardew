#include "furniture_view.h"

FurnitureView::FurnitureView(const FurnitureData& data) {
    update(data);
}

void FurnitureView::update(const FurnitureData& data) {
    pos = data.pos;
    type = data.type;
    dir = data.dir;
}

std::optional<float> FurnitureView::depth(const RenderInfo& ctx) const {
    const Sprite& sprite = ctx.assets.get_sprite(type, dir).get_sprite();
    return depth_if_visible(ctx, pos, sprite.get_height(), is_flat(type) ? -sprite.get_height() : 0);
}

void FurnitureView::draw(const RenderInfo& ctx) {
    ctx.assets.get_sprite(type, dir).draw_at(ctx, pos);
}