#pragma once

#include <cstdint>
#include <map>

#include "../zone_view.h"
#include "level_view.h"

class MinesView : public ZoneView {
private:
    std::map<uint8_t, LevelView> levels;
    uint8_t current_level = 0;

public:
    // TODO: arreglar para que use level data
    void update(const MineData& data);
    void draw_background(const RenderInfo& ctx) override;
    void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) override;

    std::optional<ClickTarget> pick_target(int world_x, int world_y) const override;
};
