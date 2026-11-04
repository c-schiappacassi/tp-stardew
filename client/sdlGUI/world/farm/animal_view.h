#pragma once
#include <optional>

#include "../../utils.h"
#include "../entity_view.h"
#include "../../animator.h"
#include "../../render_info.h"
#include "../../draw_region.h"

#include "position.h"
#include "protocol_types.h"
#include "constants.h"
#include "dtos.h"

namespace dimentions {
    const int COW_H = 32;
    const int CHICKEN_H = 16;
}

class AnimalView: public EntityView {
private:
    Position pos{0, 0};
    AnimalType type;
    bool needs_interaction;
    Direction facing = Direction::Down;
    AnimalState state;
    
    Animator anim;
    std::optional<ClipId> current_clip;
    long long last_it = -1;
public:
    explicit AnimalView(const AnimalData& data);

    void update(const AnimalData& data);

    std::optional<float> depth(const RenderInfo& ctx) const override;
    void draw(const RenderInfo& ctx) override;
};