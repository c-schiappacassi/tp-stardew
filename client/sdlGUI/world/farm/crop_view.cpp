#include "crop_view.h"


CropView::CropView(const CropData& data) {
    update(data);
}

std::optional<float> CropView::depth(const RenderInfo& ctx) const {
    return depth_if_visible(ctx, pos, crop::CROP_HEIGHT);
};

void CropView::draw(const RenderInfo& ctx){
    const Sprite& sprite = ctx.assets.get_sprite(this->type, this->state );
    draw_sprite_at(ctx, sprite, pos, false, -crop::CROP_Y_OFFSET);
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