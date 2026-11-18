#include "item_view.h"

ItemView::ItemView(const LootData& data) {
    update(data);
}

void ItemView::update(const LootData& data) {
    pos = data.pos;
    item = data.kind == LootKind::Gold ? ItemId::Gold : data.item;
}

std::optional<float> ItemView::depth(const RenderInfo& ctx) const {
    const int radius = TILE_TO_PIXEL * ctx.cam.get_zoom();

    if (!ctx.cam.is_visible(pos.getX(), pos.getY(), radius)) {
        return std::nullopt;
    }

    return static_cast<float>(pos.getY() - TILE_TO_PIXEL);
}

void ItemView::draw(const RenderInfo& ctx) {
    const Sprite* icon = ctx.assets.find_sprite(item);
    if (!icon) {
        return;
    }

    const int tile = TILE_TO_PIXEL * ctx.cam.get_zoom();
    const SDL2pp::Point top_left = ctx.cam.world_to_screen(pos.getX() - TILE_TO_PIXEL / 2, pos.getY() - TILE_TO_PIXEL);
    const SDL2pp::Rect tile_rect(top_left.x, top_left.y, tile, tile);

    draw_sprite_fit(ctx, *icon, tile_rect, tile);
}