#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "background.h"
#include "item_view.h"
#include "zone_view.h"
#include "../sync_views.h"

class BackgroundZoneView : public ZoneView {
private:
    Background back;
    std::unordered_map<uint32_t, ItemView> loot;

protected:
    BackgroundZoneView(const BackgroundData& data, AssetManager& assets): back(data, assets) {}

    virtual void collect_entities(const RenderInfo& ctx, std::vector<Drawable>& out, uint64_t& tie) = 0;
    virtual void draw_floor_overlay(const RenderInfo&) {}; 

    void update_loot(const std::vector<LootData>& data) {
        sync_views(loot, data, [](const LootData& l) { return l.id; });
    }
public:
    void draw_background(const RenderInfo& ctx) final { 
        back.draw(ctx);
        draw_floor_overlay(ctx);
    }

    void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) final {
        uint64_t tie = ZONE_TIEBREAK_BASE;
        back.collect_drawables(ctx, out, tie);
        collect_from(ctx, loot, out, tie);
        collect_entities(ctx, out, tie);
    }
};