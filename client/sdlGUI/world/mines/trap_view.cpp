#include "trap_view.h"

#include "../../draw_region.h"
#include "../../render_info.h"
#include "../../sprite_defs.h"
#include "constants.h"

namespace {
    // TODO: tambien como en enemigo pasar esto a asset manager
    SpriteId sprite_for(TrapType type) {
        switch (type) {
            case TrapType::Spikes:    return SpriteId::SpikeTrap;
            case TrapType::WallFlame: return SpriteId::WallFlameTrap;
        }
        return SpriteId::SpikeTrap;
    }

    Direction clip_dir(TrapType type, Direction facing) {
        return type == TrapType::Spikes ? Direction::None : facing;
    }

    int height_for(TrapType type) {
        return type == TrapType::Spikes ? Dimentions::SPIKE_CELL_H : Dimentions::WALL_FLAME_MAX_H;
    }
}

TrapView::TrapView(const TrapData& data) {
    update(data);
}

void TrapView::update(const TrapData& data) {
    pos = data.pos;
    type = data.type;
    state = data.state;

    if (data.dir != Direction::None) {
        facing = data.dir;
    }
}

std::optional<float> TrapView::depth(const RenderInfo& ctx) const {
    return depth_if_visible(ctx, pos, height_for(type), type == TrapType::Spikes ? -TILE_TO_PIXEL : 0);
}

void TrapView::draw(const RenderInfo& ctx) {
    draw_clip_at(ctx, clip_player, ClipId{sprite_for(type), Action{state}, clip_dir(type, facing)}, pos);
}
