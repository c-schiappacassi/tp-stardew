#pragma once
#include "../background/background_zone_view.h"
#include "../background/furniture_view.h"
#include "../../sync_views.h"

class InteriorView : public BackgroundZoneView {
    std::unordered_map<uint32_t, FurnitureView> furniture;

protected:
    void collect_entities(const RenderInfo& ctx, std::vector<Drawable>& out, uint64_t& tie) override {
        collect_from(ctx, furniture, out, tie);
    }
public:
    InteriorView(const BackgroundData& d, AssetManager& a): BackgroundZoneView(d, a) {}
    void update(const InteriorData& d) {
        sync_views(furniture, d.furniture, [](const FurnitureData& f) { return f.id; });
    }
};