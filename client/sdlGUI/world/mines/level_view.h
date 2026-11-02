#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include "../background.h"
#include "../zone_view.h"

#include "ore_view.h"
#include "position.h"


class LevelView: public ZoneView {
    Background map;
    uint8_t number;
    Position stairs_up, stairs_down;
    std::unordered_map<uint32_t, OreView> ores;
    // TODO:
    // std::unordered_map<uint32_t, Enemy> enemies;
    // std::vector<Chest> chests;
    // std::vector<Trap>  traps;

public:
    // TODO:
    explicit LevelView(uint8_t number): number(number), stairs_up(Position(0, 0)), stairs_down(Position(0, 0)){};

    // TODO: areglar para usar LevelData
    void update(const std::vector<MineralData>& minerals);
    void draw_background(const RenderInfo& ctx) override;
    void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) override;

    std::optional<ClickTarget> pick_target(int world_x, int world_y) const override;
};