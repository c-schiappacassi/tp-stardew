#pragma once

#include <map>
#include <memory>
#include <vector>

#include "zone_view.h"
#include "protocol_types.h"
#include "mines/mine_view.h"

class MapView : public ZoneView {
private:
    Zone current = Zone::Farm;
    MinesView mines;
    std::map<Zone, std::unique_ptr<ZoneView>> zones;

    ZoneView* current_view() {
        if (current == Zone::Dungeon) return &mines;
        const auto it = zones.find(current);
        return it == zones.end() ? nullptr : it->second.get();
    }

    const ZoneView* current_view() const {
        if (current == Zone::Dungeon) return &mines;
        const auto it = zones.find(current);
        return it == zones.end() ? nullptr : it->second.get();
    }

public:
    void update(const State& data) {
        const PlayerDataLocal& local = data.get_local();
        current = local.currentZone ;

        if (current == Zone::Dungeon) {
            if (const auto* mine = std::get_if<MineData>(&local.zoneData)) {
                mines.update(*mine);
            }
        }
    }

    void draw_background(const RenderInfo& ctx) override {
        if (ZoneView* view = current_view()) view->draw_background(ctx);
    }

    void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) override {
        if (ZoneView* view = current_view()) view->collect_drawables(ctx, out);
    }

    std::optional<ClickTarget> pick_target(int world_x, int world_y) const override {
        const ZoneView* view = current_view();
        return view ? view->pick_target(world_x, world_y) : std::nullopt;
    }
};
