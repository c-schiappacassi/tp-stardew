#pragma once
#include "../zone_view.h"
#include "house_view.h"

class HousesView : public ZoneView {
    std::map<uint8_t, HouseView> houses;
    uint8_t current_house = 0;
public:
    HousesView(const std::vector<BackgroundData>& bgs, AssetManager& assets) {
        for (std::size_t i = 0; i < bgs.size(); ++i)
            houses.try_emplace(static_cast<uint8_t>(i), bgs[i], assets);
    }

    void update(const HouseData& d) {
        current_house = d.house_index;
        if (auto it = houses.find(current_house); it != houses.end())
            it->second.update(d);
    }
    void draw_background(const RenderInfo& ctx) override { houses.at(current_house).draw_background(ctx); }
    void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) override {  houses.at(current_house).collect_drawables(ctx, out); }
};