#include "enemy_view.h"

#include <cmath>

#include "../../draw_region.h"
#include "../../render_info.h"
#include "../../sprite_defs.h"
#include "constants.h"

// TODO: pasar esto a asset manager con get_sprite_id(EnemyType type, bool frenzied) o algo asi, tambien usar sprite.height para depth
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
    return depth_if_visible(ctx, pos, height_for(type));
}

void EnemyView::draw(const RenderInfo& ctx) {
    const auto sprite_id = sprite_for(type, frenzied);
    if (!sprite_id) {
        return;
    }

    draw_clip_at(ctx, clip_player, ClipId{*sprite_id, Action{state}, facing}, pos, type == EnemyType::Skull ? bob_offset(ctx) : 0);
}