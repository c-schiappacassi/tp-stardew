#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include "../zone_view.h"

#include "dtos.h"
#include "level_view.h"

class MinesView: public ZoneView {
    std::unordered_map<uint8_t, LevelView> levels;
    uint8_t current_level = 0;

public:
    void update(const MineData& data);
    void draw_background(const RenderInfo& ctx) override;
    void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) override;

    std::optional<ClickTarget> pick_target(int world_x, int world_y) const override {
        auto it = levels.find(current_level);
        if (it == levels.end()) {
            return std::nullopt;
        }
        return it->second.pick_target(world_x, world_y);
    }
};