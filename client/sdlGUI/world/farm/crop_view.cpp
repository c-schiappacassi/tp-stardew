#include "crop_view.h"


CropView::CropView(const CropData& data) {
    crop_trigger.add<WateredSound>(sfx);
    update(data);
}

std::optional<float> CropView::depth(const RenderInfo& ctx) const {
    return depth_if_visible(ctx, pos, crop::CROP_HEIGHT);
};

void CropView::draw(const RenderInfo& ctx){
    sfx.flush(ctx);

    const Sprite& sprite = ctx.assets.get_sprite(this->type, this->state );
    sprite.draw_at(ctx, pos, {.world_dy = -crop::CROP_Y_OFFSET});
};

void CropView::update(const CropData& data){
    sfx.clear();
    this->pos = data.pos;
    this->type = data.type;
    this->state = data.state;
    this->wattered = data.wattered;
    crop_trigger.observe(data.wattered, data.type, data.pos);
}

std::optional<CropType> CropView::is_ready(){
    if (this->state == CropState::Ready){
        return this->type;
    }
    return std::nullopt;
}