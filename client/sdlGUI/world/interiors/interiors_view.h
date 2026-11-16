#pragma once
#include "../zone_view.h"
#include "interior_view.h"

class InteriorsView : public ZoneView {
    std::map<uint8_t, InteriorView> houses;
    std::map<InteriorId, InteriorView> shops;
    InteriorView* current = nullptr;

    InteriorView* find_view(const InteriorData& d) {
        if (d.interior_id == InteriorId::PlayerHouse) {
            const auto it = houses.find(d.house_index);
            return it == houses.end() ? nullptr : &it->second;
        }
        const auto it = shops.find(d.interior_id);
        return it == shops.end() ? nullptr : &it->second;
    }

public:
     InteriorsView(const ConstMapData& data, AssetManager& assets) {
        for (std::size_t i = 0; i < data.farm_houses.size(); ++i){
            houses.try_emplace(static_cast<uint8_t>(i), data.farm_houses[i], assets);
        }
        shops.try_emplace(InteriorId::PierresShop, data.pierres, assets);
        shops.try_emplace(InteriorId::RobinsShop, data.robins, assets);
        shops.try_emplace(InteriorId::ClintsShop, data.clints, assets);
        shops.try_emplace(InteriorId::MarniesShop, data.marnies, assets);
    }

    void update(const InteriorData& d) {
        current = find_view(d);
        if (current) current->update(d);
    }
    void draw_background(const RenderInfo& ctx) override {
        if (current) current->draw_background(ctx);
    }
    void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) override {
        if (current) current->collect_drawables(ctx, out);
    }
};