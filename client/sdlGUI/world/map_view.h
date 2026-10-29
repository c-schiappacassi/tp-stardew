
#include "zone_view.h"

class MapView : public ZoneView
{
private:
public:
    void update(const State& data);
    void draw_background(const RenderInfo& ctx) override;
    void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) override;
};
