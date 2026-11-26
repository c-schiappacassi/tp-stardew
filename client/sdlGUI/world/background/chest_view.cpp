#include "chest_view.h"

namespace {
    const int CHEST_CELL_H = 32;
    ClipId clip_of(ChestState state) {
        return ClipId{SpriteId::Craftables, Action{state}, Direction::None};
    }
}

ChestView::ChestView(const FurnitureData& data) {
    chest_trigger.add<ChestSound>(sfx);
    update(data);
}

void ChestView::update(const FurnitureData& data) {
    id = data.id;
    pos = data.pos;
}

bool ChestView::is_open(const RenderInfo& ctx) const {
    const auto& open = ctx.state.get_local().openChest;
    return open.has_value() && open->id == id;
}

std::optional<float> ChestView::depth(const RenderInfo& ctx) const {
    return depth_if_visible(ctx, pos, CHEST_CELL_H);
}

void ChestView::draw(const RenderInfo& ctx) {
    const bool open = is_open(ctx);
    chest_trigger.observe(open, pos);
    sfx.flush(ctx);

    if (open) {
        state = ChestState::Opening;
    } else if (state == ChestState::Opening) {
        state = ChestState::Closing;
    }
    
    player.draw_at(ctx, clip_of(state), pos);
}
