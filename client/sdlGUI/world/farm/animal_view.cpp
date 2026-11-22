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
    update(data);
}

void AnimalView::update(const AnimalData& data) {
    pos = data.pos;
    type = data.type;
    needs_interaction = data.needs_interaction;
    facing = data.dir;
    state = data.state;
}

void AnimalView::draw(const RenderInfo& ctx) {
    const ClipId id{ctx.assets.get_sprite_id(type), Action{state}, facing};

    const AnimatedFrame* frame = draw_clip_at(ctx, clip_player, id, pos);
    if (!frame) {
        return;
    }

    if (this->needs_interaction){
        SDL2pp::Point origin_emote = ctx.cam.world_to_screen(pos.getX() , pos.getY() - (frame->sprite.height * 0.75));
        const Sprite& emote = ctx.assets.get_sprite(EmoteId::Alert);
        draw_sprite(ctx, emote, origin_emote);
    }
}