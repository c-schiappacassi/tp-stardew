#pragma once

#include <vector>

#include "zone_view.h"
#include "farm/farm_view.h"
#include "protocol_types.h"
#include "mines/mine_view.h"
#include "town/town_view.h"

class MapView : public ZoneView {
private:
    Zone current = Zone::Farm;
    // TODO: mover las Views al map cuando tenga todo para el constructor(Background incluido)
    MinesView mines;
    FarmView farm;
    TownView town;

    ZoneView* current_view() {
        if (current == Zone::Dungeon) return &mines;
        if (current == Zone::Farm) return &farm;
        if (current == Zone::Town) return &town;
        return nullptr;
    }

public:
    MapView(const ConstMapData& data, AssetManager& assets) : mines(data.levels, assets), farm(data.farm, assets), town(data.town, assets) {}

    void update(const State& data) {
        const PlayerDataLocal& local = data.get_local();
        current = local.currentZone ;

        if (const auto* mine = std::get_if<MineData>(&local.zoneData)) {
            mines.update(*mine);
        } else if (const auto* f = std::get_if<FarmData>(&local.zoneData)) {
            farm.update(*f);
        }
    }

    void draw_background(const RenderInfo& ctx) override {
        if (ZoneView* view = current_view()) view->draw_background(ctx);
    }

    void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) override {
        if (ZoneView* view = current_view()) view->collect_drawables(ctx, out);
    }
};