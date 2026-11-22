#pragma once
#include <optional>

#include "../../utils.h"
#include "../entity_view.h"
#include "../../clip_player.h"

#include "position.h"
#include "protocol_types.h"
#include "dtos.h"

class TrapView: public EntityView {
private:
    Position pos{0, 0};
    TrapType type;
       
    Direction facing = Direction::Down;
    TrapState state;

    ClipPlayer clip_player;

public:
    explicit TrapView(const TrapData& data);

    void update(const TrapData& data);

    std::optional<float> depth(const RenderInfo& ctx) const override;
    void draw(const RenderInfo& ctx) override;
};