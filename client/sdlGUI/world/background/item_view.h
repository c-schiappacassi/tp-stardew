#pragma once
#include <optional>

#include "../../utils.h"
#include "../entity_view.h"
#include "../../render_info.h"
#include "../../draw_region.h"

#include "position.h"
#include "protocol_types.h"
#include "dtos.h"

class ItemView : public EntityView {
private:
    Position pos{0, 0};
    ItemId item = ItemId::Silver;

public:
    explicit ItemView(const LootData& data);

    void update(const LootData& data);

    std::optional<float> depth(const RenderInfo& ctx) const override;
    void draw(const RenderInfo& ctx) override;
};