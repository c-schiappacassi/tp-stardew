#pragma once
#include <vector>

#include "background.h"
#include "zone_view.h"

class BackgroundZoneView : public ZoneView {
private:
    Background back;

protected:
    BackgroundZoneView(const BackgroundData& data, AssetManager& assets): back(data, assets) {}

    virtual void collect_entities(const RenderInfo& ctx, std::vector<Drawable>& out, uint64_t& tie) = 0;

public:
    void draw_background(const RenderInfo& ctx) final { back.draw(ctx); }

    void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) final {
        uint64_t tie = ZONE_TIEBREAK_BASE;
        back.collect_drawables(ctx, out, tie);
        collect_entities(ctx, out, tie);
    }
};