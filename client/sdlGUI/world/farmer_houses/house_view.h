#pragma once
#include "../background_zone_view.h"
#include "../furniture_view.h"
#include "../../sync_views.h"


class HouseView : public BackgroundZoneView {
    std::unordered_map<uint32_t, FurnitureView> furniture;
protected:
    void collect_entities(const RenderInfo& ctx, std::vector<Drawable>& out, uint64_t& tie) override {
        collect_from(ctx, furniture, out, tie);
    }
public:
    HouseView(const BackgroundData& d, AssetManager& a): BackgroundZoneView(d, a) {}
    void update(const HouseData& d) {
        sync_views(furniture, d.furniture, [](const FurnitureData& f) { return f.id; });
    }
};