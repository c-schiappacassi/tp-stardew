#include "../zone_view.h"
#include "level_view.h"

class MinesView : public ZoneView {
    std::unordered_map<uint8_t, LevelView> levels;

public:
    void update(const MineData& data);
    void draw_background(const RenderInfo& ctx) override;
    void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) override;
};