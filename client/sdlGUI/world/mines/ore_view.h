#pragma once
#include <cstdint>
#include <optional>

#include "../../utils.h"
#include "../entity_view.h"

#include "dtos.h"
#include "position.h"
#include "protocol_types.h"

class OreView: public EntityView {
private:
    uint32_t entityId;
    Position pos{0, 0};
    ResourceType type;
    ResourceState state;
    // TODO: De MineralState a ResourseState

public:
    explicit OreView(const MineralData& data);

    void update(const MineralData& data);

    uint32_t id() const { return entityId; }

    bool is_hit(int world_x, int world_y) const;

    std::optional<float> depth(const RenderInfo& ctx) const override;
    void draw(const RenderInfo& ctx) override;
};