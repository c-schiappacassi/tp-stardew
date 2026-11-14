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
    const Sprite& sprite = ctx.assets.get_sprite(type, dir);
    const int radius = sprite.height * ctx.cam.get_zoom();

    if (!ctx.cam.is_visible(pos.getX(), pos.getY(), radius)) {
        return std::nullopt;
    }

    const float y = static_cast<float>(pos.getY());
    return is_flat(type) ? y - sprite.height : y;
}

void FurnitureView::draw(const RenderInfo& ctx) {
    const Sprite& sprite = ctx.assets.get_sprite(type, dir);
    draw_sprite(ctx, sprite, ctx.cam.world_to_screen(pos.getX(), pos.getY()));
}