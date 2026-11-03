#pragma once
#include <optional>

#include "../../utils.h"
#include "../entity_view.h"
#include "../../animator.h"

#include "position.h"
#include "protocol_types.h"
#include "dtos.h"


class AnimalView: public EntityView {
private:
    Position pos{0, 0};
    AnimalType type;
       
    Direction facing = Direction::Down;
    AnimalState state;
    
    Animator anim;
    std::optional<ClipId> current_clip;
    long long last_it = -1;

    // age ? TBD

public:
    explicit AnimalView(const AnimalData& data);

    void update(const AnimalData& data);

    std::optional<float> depth(const RenderInfo& ctx) const override;
    void draw(const RenderInfo& ctx) override;
};