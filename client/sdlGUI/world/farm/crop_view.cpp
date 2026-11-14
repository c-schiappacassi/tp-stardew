#include "crop_view.h"


CropView::CropView(const CropData& data) {
    update(data);
}

std::optional<float> CropView::depth(const RenderInfo& ctx) const {
    const int radius = crop::HEIGHT * ctx.cam.get_zoom();

    if (!ctx.cam.is_visible(pos.getX(), pos.getY(), radius)) {
        return std::nullopt;
    }

    return static_cast<float>(pos.getY());
};

void CropView::draw(const RenderInfo& ctx){
    const Sprite& sprite = ctx.assets.get_sprite(this->type, this->state );
    const SDL2pp::Point origin = ctx.cam.world_to_screen(pos.getX(), pos.getY());
    draw_sprite(ctx, sprite, origin);
};

void CropView::update(const CropData& data){
    this->pos = data.pos;
    this->type = data.type;
    this->state = data.state;
    this->wattered = data.wattered;
}

std::optional<CropType> CropView::is_ready(){
    if (this->state == CropState::Ready){
        return this->type;
    }
    return std::nullopt;
}