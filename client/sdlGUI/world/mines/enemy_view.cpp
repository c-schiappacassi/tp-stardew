#include "enemy_view.h"

#include <cmath>

#include "../../draw_region.h"
#include "../../render_info.h"
#include "../../sprite_defs.h"
#include "constants.h"

namespace {
    std::optional<SpriteId> sprite_for(EnemyType type, bool frenzied) {
        switch (type) {
            case EnemyType::Skeleton: return SpriteId::Skeleton;
            case EnemyType::Orc:      return SpriteId::StoneGolem;
            case EnemyType::VampireMale:   return frenzied ? SpriteId::FrostBatFrenzy : SpriteId::FrostBat;
            case EnemyType::VampireFemale: return frenzied ? SpriteId::LavaBatFrenzy  : SpriteId::LavaBat;
            case EnemyType::Skull:         return SpriteId::Skull;
            case EnemyType::Traps:    return std::nullopt;
        }
        return std::nullopt;
    }

    int hover_bob(EnemyType type, long long it) {
        if (type != EnemyType::Skull) return 0;
        return static_cast<int>(std::lround(std::sin(static_cast<double>(it) * 0.15) * 2.0));
    }

    int height_for(EnemyType type) {
        switch (type) {
            case EnemyType::Skeleton: return Dimentions::SKELETON_CELL_H;
            case EnemyType::Orc:      return Dimentions::GOLEM_CELL_H;
            case EnemyType::VampireMale:
            case EnemyType::VampireFemale: return Dimentions::BAT_CELL_H;
            case EnemyType::Skull:         return Dimentions::SKULL_CELL_H;
            default:                  return Dimentions::SKELETON_CELL_H;
        }
    }
}

EnemyView::EnemyView(const EnemySnapshot& data) { update(data); }

void EnemyView::update(const EnemySnapshot& data) {
    pos = data.pos;
    type = data.type;
    state = data.state;
    frenzied = data.frenzied && data.state != EnemyState::Dead;

    if (data.dir != Direction::None) {
        facing = data.dir;
    }
}

std::optional<float> EnemyView::depth(const RenderInfo& ctx) const {
    if (!sprite_for(type, frenzied)) {
        return std::nullopt;
    }

    const int radius = height_for(type) * ctx.cam.get_zoom();
    if (!ctx.cam.is_visible(pos.getX(), pos.getY(), radius)) {
        return std::nullopt;
    }

    return static_cast<float>(pos.getY());
}

void EnemyView::draw(const RenderInfo& ctx) {
    const auto sprite_id = sprite_for(type, frenzied);
    if (!sprite_id) {
        return;
    }

    const ClipId id{*sprite_id, Action{state}, facing};

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

    SDL2pp::Point origin = ctx.cam.world_to_screen(pos.getX(), pos.getY());
    origin.y += hover_bob(type, ctx.it) * ctx.cam.get_zoom();
    draw_sprite(ctx, frame.sprite, origin, frame.flip);

}