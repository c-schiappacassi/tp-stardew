#pragma once

#include <cstdint>
#include <map>

#include "../zone_view.h"
#include "../lighting.h"
#include "level_view.h"


class MinesView : public ZoneView {
private:
    std::map<uint8_t, LevelView> levels;
    uint8_t current_level = 0;

public:
    MinesView(const std::vector<BackgroundData>& level_backgrounds, AssetManager& assets) {
        for (std::size_t i = 0; i < level_backgrounds.size(); ++i) {
            const auto number = static_cast<uint8_t>(i + 1);
            levels.try_emplace(number, number, level_backgrounds[i], assets);
        }
    }

    void update(const MineData& data);  
    void draw_background(const RenderInfo& ctx) override;
    void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) override;
    SDL_Color ambient_light(const RenderInfo& ctx) override;
};