#include "animal_view.h"

namespace {
    // TODO: cambiar sprite dependiendo del color de vaca y pollo
    SpriteId sprite_for(AnimalType type) {
        switch (type) {
            case AnimalType::BrownCow:     return SpriteId::BronwCow;
            case AnimalType::WhiteCow:     return SpriteId::BronwCow;
            case AnimalType::BrownChicken: return SpriteId::BronwChicken;
            case AnimalType::WhiteChicken: return SpriteId::BronwChicken;
            case AnimalType::BlueChicken: return SpriteId::BronwChicken;
            case AnimalType::VoidChicken: return SpriteId::BronwChicken;
        }
        throw std::invalid_argument("AnimalView: AnimalType desconocido");
    }
}  

std::optional<float> AnimalView::depth(const RenderInfo& ctx) const {
    int heigt = this->type == AnimalType::BrownCow ? dimentions::COW_H : dimentions::CHICKEN_H;
    const int radius = heigt * ctx.cam.get_zoom();

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
    const SpriteId sprite_id = sprite_for(type);
    const ClipId id{sprite_id, Action{state}, facing};

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

    // TODO: if (needs_interaction) dibujar el indicador
}