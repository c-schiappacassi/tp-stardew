// sdlGUI/world/furniture_view.h
#pragma once
#include <optional>

#include "../utils.h"
#include "entity_view.h"
#include "../render_info.h"
#include "../draw_region.h"

#include "position.h"
#include "protocol_types.h"
#include "dtos.h"

class FurnitureView: public EntityView {
private:
    Position pos{0, 0};
    FurnitureId type = FurnitureId::Chair;
    Direction dir = Direction::Down;

    static bool is_flat(FurnitureId id) {
        return id == FurnitureId::BeigeRug || id == FurnitureId::BlueRug || id == FurnitureId::Bed;
    }

public:
    explicit FurnitureView(const FurnitureData& data);

    void update(const FurnitureData& data);

    std::optional<float> depth(const RenderInfo& ctx) const override;
    void draw(const RenderInfo& ctx) override;
};