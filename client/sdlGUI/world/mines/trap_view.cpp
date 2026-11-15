#include "trap_view.h"

#include "../../draw_region.h"
#include "../../render_info.h"
#include "../../sprite_defs.h"
#include "constants.h"

namespace {
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
    const int radius = height_for(type) * ctx.cam.get_zoom();
    if (!ctx.cam.is_visible(pos.getX(), pos.getY(), radius)) {
        return std::nullopt;
    }

    if (type == TrapType::Spikes) {
        return static_cast<float>(pos.getY() - TILE_TO_PIXEL);
    }

    return static_cast<float>(pos.getY());
}

void TrapView::draw(const RenderInfo& ctx) {
    const ClipId id{sprite_for(type), Action{state}, clip_dir(type, facing)};

    if (!current_clip || !(*current_clip == id)) {
        current_clip = id;
        anim.reset();
    }

    if (last_it >= 0 && ctx.it > last_it) {
        anim.update(static_cast<float>(ctx.it - last_it) * TICK_MS);
    }
    last_it = ctx.it;

    if (!ctx.assets.has_clip(id)) {
        return;
    }

    const Clip& clip = ctx.assets.get_clip(id);
    const AnimatedFrame& frame = anim.get_current_frame(clip);

    const SDL2pp::Point origin = ctx.cam.world_to_screen(pos.getX(), pos.getY());
    draw_sprite(ctx, frame.sprite, origin, frame.flip);
}
