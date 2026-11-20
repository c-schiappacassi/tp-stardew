#pragma once

#include <cstdint>
#include <vector>

#include "../background_zone_view.h"
#include "../lighting.h"   

class TownView : public BackgroundZoneView {
protected:
    void collect_entities(const RenderInfo&, std::vector<Drawable>&, uint64_t&) override {}
    SDL_Color ambient_light(const RenderInfo& ctx) override { return lighting::outdoor_ambient(ctx); }

public:
    TownView(const BackgroundData& data, AssetManager& assets): BackgroundZoneView(data, assets) {}
};
