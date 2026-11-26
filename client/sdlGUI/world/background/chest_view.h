#pragma once
#include <cstdint>
#include <optional>

#include "../../utils.h"
#include "../../clip_player.h"
#include "../entity_view.h"
#include "../../render_info.h"
#include "../../draw_helpers.h"
#include "../effects/sounds/chest_sound.h"
#include "../effects/triggers/chest_trigger.h"

#include "position.h"
#include "protocol_types.h"
#include "dtos.h"

class ChestView: public EntityView {
private:
    uint32_t id = 0;
    Position pos{0, 0};

    ClipPlayer player;
    ChestState state = ChestState::Closed;
    SoundEmitter sfx;
    ChestTrigger chest_trigger;

    bool is_open(const RenderInfo& ctx) const;

public:
    explicit ChestView(const FurnitureData& data);

    void update(const FurnitureData& data);

    std::optional<float> depth(const RenderInfo& ctx) const override;
    void draw(const RenderInfo& ctx) override;
};
