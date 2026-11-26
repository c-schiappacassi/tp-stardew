#include "animal_view.h"

namespace {
    bool is_cow(AnimalType t) {
        return t == AnimalType::BrownCow || t == AnimalType::WhiteCow;
    }
}

std::optional<float> AnimalView::depth(const RenderInfo& ctx) const {
    const int height = is_cow(type) ? dimentions::COW_H : dimentions::CHICKEN_H;
    return depth_if_visible(ctx, pos, height);
};

AnimalView::AnimalView(const AnimalData& data) {
    call_trigger.add<AnimalCallSound>(sfx);
    update(data);
}

void AnimalView::update(const AnimalData& data) {
    sfx.clear();
    pos = data.pos;
    type = data.type;
    needs_interaction = data.needs_interaction;
    facing = data.dir;
    state = data.state;
    call_trigger.observe(data.type, data.pos);
}

void AnimalView::draw(const RenderInfo& ctx) {
    sfx.flush(ctx);

    const ClipId id{ctx.assets.get_sprite_id(type), Action{state}, facing};

    const AnimatedFrame* frame = clip_player.draw_at(ctx, id, pos);
    if (!frame) {
        return;
    }

    if (this->needs_interaction){
        SDL2pp::Point origin_emote = ctx.cam.world_to_screen(pos.getX() , pos.getY() - (frame->get_sprite().get_height() * 0.75));
        const Sprite& emote = ctx.assets.get_sprite(EmoteId::Alert);
        emote.draw(ctx, origin_emote);
    }
}