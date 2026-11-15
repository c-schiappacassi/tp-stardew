#pragma once
#include <optional>

#include "../../utils.h"
#include "../entity_view.h"
#include "../../animator.h"

#include "position.h"
#include "protocol_types.h"
#include "dtos.h"

class ProjectileView: public EntityView {
private:
    Position pos{0, 0};
    FlameType type;

    Animator anim;
    std::optional<ClipId> current_clip;
    long long last_it = -1;

public:
    explicit ProjectileView(const ProjectileData& data);
    void update(const ProjectileData& data);

    std::optional<float> depth(const RenderInfo& ctx) const override;
    void draw(const RenderInfo& ctx) override;
};