#pragma once
#include <optional>

#include "../../utils.h"
#include "../entity_view.h"
#include "../../render_info.h"
#include "../../draw_region.h"

#include "position.h"
#include "protocol_types.h"
#include "dtos.h"


namespace crop {
    const int HEIGHT = 32;
}


class CropView: public EntityView {
private:
    Position pos{0, 0};
    CropType type;
    CropState state;
    bool wattered;

public:
    explicit CropView(const CropData& data);

    void update(const CropData& data);

    std::optional<float> depth(const RenderInfo& ctx) const override;

    void draw(const RenderInfo& ctx) override;

    std::optional<CropType> is_ready();
};

