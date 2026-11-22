#pragma once
#include <optional>

#include "../../utils.h"
#include "../entity_view.h"
#include "../../clip_player.h"

#include "position.h"
#include "protocol_types.h"
#include "dtos.h"

namespace dimentions {
    constexpr int FLAME_SIZE = 8;
}

class ProjectileView: public EntityView {
private:
    Position pos{0, 0};
    FlameType type;

    ClipPlayer clip_player;

public:
    explicit ProjectileView(const ProjectileData& data);
    void update(const ProjectileData& data);

    std::optional<float> depth(const RenderInfo& ctx) const override;
    void draw(const RenderInfo& ctx) override;
};