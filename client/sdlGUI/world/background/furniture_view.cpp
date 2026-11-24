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
    const Sprite& sprite = ctx.assets.get_sprite(type, dir).sprite;
    return depth_if_visible(ctx, pos, sprite.height, is_flat(type) ? -sprite.height : 0);
}

void FurnitureView::draw(const RenderInfo& ctx) {
    const FlippableSprite& f = ctx.assets.get_sprite(type, dir);
    draw_sprite_at(ctx, f.sprite, pos, f.flip);
}