#pragma once
#include <map>
#include <memory>
#include <optional>

#include "zone_view.h"
#include "protocol_types.h"

class MapView: public ZoneView {
private:
    Zone current = Zone::Farm;
    std::map<Zone, std::unique_ptr<ZoneView>> zones;

public:
    void update(const State& data);
    void draw_background(const RenderInfo& ctx) override;
    void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) override;

    std::optional<ClickTarget> pick_target(int world_x, int world_y) const override {
        auto it = zones.find(current);
        if (it == zones.end()) {
            return std::nullopt;
        }
        return it->second->pick_target(world_x, world_y);
    }
};