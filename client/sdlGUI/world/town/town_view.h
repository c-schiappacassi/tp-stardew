#pragma once

#include <cstdint>
#include <vector>

#include "../background_zone_view.h"

class TownView : public BackgroundZoneView {
protected:
    void collect_entities(const RenderInfo&, std::vector<Drawable>&, uint64_t&) override {}

public:
    TownView(const BackgroundData& data, AssetManager& assets): BackgroundZoneView(data, assets) {}
};
