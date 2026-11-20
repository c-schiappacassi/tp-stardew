#include "item_view.h"

ItemView::ItemView(const LootData& data) {
    update(data);
}

void ItemView::update(const LootData& data) {
    pos = data.pos;
    item = data.kind == LootKind::Gold ? ItemId::Gold : data.item;
}

std::optional<float> ItemView::depth(const RenderInfo& ctx) const {
    return depth_if_visible(ctx, pos, TILE_TO_PIXEL, -TILE_TO_PIXEL);
}

void ItemView::draw(const RenderInfo& ctx) {
    const int tile = TILE_TO_PIXEL * ctx.cam.get_zoom();
    const SDL2pp::Point top_left = ctx.cam.world_to_screen(pos.getX() - TILE_TO_PIXEL / 2, pos.getY() - TILE_TO_PIXEL);

    draw_item_icon(ctx, item, SDL2pp::Rect(top_left.x, top_left.y, tile, tile), tile);
}