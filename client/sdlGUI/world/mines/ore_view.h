#pragma once
#include <cstdint>
#include <optional>

#include "../../utils.h"
#include "../entity_view.h"
#include "../effects/sounds/ore_hit_sound.h"
#include "../effects/triggers/health_trigger.h"

#include "dtos.h"
#include "position.h"
#include "protocol_types.h"

class OreView: public EntityView {
private:
    Position pos{0, 0};
    ResourceType type;
    ResourceState state;
    SoundEmitter sfx;
    HealthTrigger health_trigger;

public:
    explicit OreView(const MineralData& data);

    void update(const MineralData& data);

    std::optional<float> depth(const RenderInfo& ctx) const override;
    void draw(const RenderInfo& ctx) override;
};