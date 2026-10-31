#pragma once
#include <cstdint>
#include <optional>

#include "../entity_view.h"
#include "../../draw_region.h"
#include "../../render_info.h"

#include "dtos.h"
#include "position.h"

class OreView: public EntityView {
private:
    uint32_t entityId;
    Position pos{0, 0};
    ResourceType type;
    ResourceState state;

public:
    OreView(uint32_t id, const OreData& data): entityId(id), type(data.type), state(data.state) {
        update(data);
    }

    void update(const OreData& data);

    bool is_hit(int world_x, int world_y) const {
        if (state == ResourceState::Broken) {
            return false;
        }
        return world_x >= pos.getX() && world_x < pos.getX() + TILE_TO_PIXEL && world_y >= pos.getY() && world_y < pos.getY() + TILE_TO_PIXEL;
    }

    std::optional<float> depth(const RenderInfo& ctx) const override;
    void OreView::draw(const RenderInfo& ctx) override {
        const Sprite& sprite = ctx.assets.get_sprite(type, state);

        const SDL2pp::Point origin = ctx.cam.world_to_screen(pos.getX(), pos.getY());
        draw_region(ctx, sprite.sprite_id, sprite_rect(sprite), origin);
    }
};