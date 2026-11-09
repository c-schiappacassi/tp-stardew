#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include "../background_zone_view.h"
#include "../../camera.h"
#include "../../sync_views.h"
#include "../../render_info.h"

#include "ore_view.h"
#include "position.h"


class LevelView : public BackgroundZoneView {
    uint8_t number;
    Position stairs_up, stairs_down;
    std::unordered_map<uint32_t, OreView> ores;
    // TODO:
    // std::unordered_map<uint32_t, EnemyView> enemies;
    // std::unordered_map<uint32_t, TrapView> traps;
    // std::unordered_map<uint32_t, ChestView> chest; // para animar que se abre, pero ver lo de adentro va a ser cosa de hud
protected:
    void collect_entities(const RenderInfo& ctx, std::vector<Drawable>& out,  uint64_t& tie) override;

public:
    LevelView(uint8_t number, const BackgroundData& data, AssetManager& assets) : BackgroundZoneView(data, assets), number(number), stairs_up(Position(0, 0)), stairs_down(Position(0, 0)) {}

    void update(const LevelData& level);
};