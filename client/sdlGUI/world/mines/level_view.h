#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include "../background.h"
#include "../zone_view.h"

#include "ore_view.h"
#include "position.h"


class LevelView: public ZoneView {
    const Background map;
    uint8_t number;
    Position stairs_up, stairs_down;
    std::unordered_map<uint32_t, OreView> ores;
    // TODO:
    // std::unordered_map<uint32_t, Enemy> enemies;
    // std::vector<Chest> chests;
    // std::vector<Trap>  traps;

public:
    // TODO:
    explicit LevelView(const Background map, uint8_t number):
            map(Background()),
            number(number),
            stairs_up(Position(0, 0)),
            stairs_down(Position(0, 0)){};

    void update(const LevelData& data);
    void draw_background(const RenderInfo& ctx) override;
    void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) override;

    std::optional<ClickTarget> pick_target(int world_x, int world_y) const override {
        for (const auto& [id, ore]: ores) {
            if (ore.is_hit(world_x, world_y)) {
                return ClickTarget{TargetType::Ore, id};
            }
        }
        return std::nullopt;
    }
};