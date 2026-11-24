#pragma once
#include "../background/background_zone_view.h"
#include "../background/furniture_views.h"
#include "../../sync_views.h"

class InteriorView : public BackgroundZoneView {
    FurnitureViews furniture;

protected:
    void collect_entities(const RenderInfo& ctx, std::vector<Drawable>& out, uint64_t& tie) override {
        furniture.collect(ctx, out, tie);
    }
public:
    InteriorView(const BackgroundData& d, AssetManager& a): BackgroundZoneView(d, a) {}
    void update(const InteriorData& d) {
        furniture.update(d.furniture);
        update_loot(d.loot);
    }
};