#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include "../background.h"
#include "../zone_view.h"
#include "../../camera.h"
#include "../../sync_views.h"
#include "../../render_info.h"

#include "ore_view.h"
#include "position.h"


class LevelView: public ZoneView {
    uint8_t number;
    Position stairs_up, stairs_down;
    std::unordered_map<uint32_t, OreView> ores;
    // TODO:
    // std::unordered_map<uint32_t, EnemyView> enemies;
    // std::unordered_map<uint32_t, TrapView> traps;
    // std::unordered_map<uint32_t, ChestView> chest; // para animar que se abre, pero ver lo de adentro va a ser cosa de hud
    // std::unordered_map<uint32_t, DecorView> decor;
public:
    // TODO:
    explicit LevelView(uint8_t number): number(number), stairs_up(Position(0, 0)), stairs_down(Position(0, 0)){};

    // TODO: areglar para usar LevelData
    void update(const  LevelData& level);
    void draw_background(const RenderInfo& ctx) override;
    void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) override;
};