#include "enemy_view.h"

#include <cmath>

#include "../../draw_region.h"
#include "../../render_info.h"

EnemyView::EnemyView(const EnemySnapshot& data) {
    hit_flash = &health_trigger.add<HitFlash>();
    health_trigger.add<EnemyHurtSound>(sfx, data.type);
    frenzy_trigger.add<VampireFrenzySound>(sfx);
    attack_trigger.add<SkullFireSound>(sfx);
    update(data);
}

void EnemyView::update(const EnemySnapshot& data) {
    sfx.clear();
    pos = data.pos;
    type = data.type;
    state = data.state;
    frenzied = data.frenzied && data.state != EnemyState::Dead;
    health_trigger.observe(data.health, data.pos);
    frenzy_trigger.observe(frenzied, data.type, data.pos);
    attack_trigger.observe(data.state, data.type, data.pos);
    if (data.dir != Direction::None) {
        facing = data.dir;
    }
}

std::optional<float> EnemyView::depth(const RenderInfo& ctx) const {
    return depth_if_visible(ctx, pos, ctx.assets.get_height(type) );
}

void EnemyView::draw(const RenderInfo& ctx) {
    sfx.flush(ctx);

    draw_clip_at(ctx, clip_player, ClipId{ctx.assets.get_sprite_id(type, frenzied), Action{state}, facing}, pos, type == EnemyType::Skull ? bob_offset(ctx) : 0, hit_flash->tint(ctx));
}