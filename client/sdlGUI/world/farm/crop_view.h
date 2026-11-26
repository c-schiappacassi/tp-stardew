#pragma once
#include <optional>

#include "../../utils.h"
#include "../entity_view.h"
#include "../../render_info.h"
#include "../../draw_region.h"
#include "../effects/sounds/watered_sound.h"
#include "../effects/triggers/crop_trigger.h"

#include "position.h"
#include "protocol_types.h"
#include "dtos.h"


namespace crop {
    const int CROP_HEIGHT = 32;
    const int CROP_Y_OFFSET = 3;
}


class CropView: public EntityView {
private:
    Position pos{0, 0};
    CropType type;
    CropState state;
    bool wattered;
    SoundEmitter sfx;
    CropTrigger crop_trigger;

public:
    explicit CropView(const CropData& data);

    void update(const CropData& data);

    std::optional<float> depth(const RenderInfo& ctx) const override;

    void draw(const RenderInfo& ctx) override;

    std::optional<CropType> is_ready();
};

