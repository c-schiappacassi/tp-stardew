#include "animal_view.h"

namespace {
    bool is_cow(AnimalType t) {
        return t == AnimalType::BrownCow || t == AnimalType::WhiteCow;
    }
}

std::optional<float> AnimalView::depth(const RenderInfo& ctx) const {
    const int height = is_cow(type) ? dimentions::COW_H : dimentions::CHICKEN_H;
    const int radius = height * ctx.cam.get_zoom();

    if (!ctx.cam.is_visible(pos.getX(), pos.getY(), radius)) {
        return std::nullopt;
    }

    return static_cast<float>(pos.getY());
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

    if (!current_clip || !(*current_clip == id)) {
        current_clip = id;
        anim.reset();
    }

    if (last_it >= 0 && ctx.it > last_it) {
        anim.update(static_cast<float>(ctx.it - last_it) * TICK_MS);
    }
    last_it = ctx.it;

    const Clip& clip = ctx.assets.get_clip(id);
    const AnimatedFrame& frame = anim.get_current_frame(clip);

    const SDL2pp::Point origin = ctx.cam.world_to_screen(pos.getX(), pos.getY());

    draw_sprite(ctx, frame.sprite, origin, frame.flip);

    if (this->needs_interaction){
        SDL2pp::Point origin_emote = ctx.cam.world_to_screen(pos.getX() , pos.getY() - frame.sprite.height);
        const Sprite& emote = ctx.assets.get_sprite(EmoteId::Alert);
        draw_sprite(ctx, emote, origin_emote);
    }
}