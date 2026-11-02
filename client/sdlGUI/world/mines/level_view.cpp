#include "level_view.h"

#include <algorithm>
#include <cstdint>

#include "../../camera.h"
#include "../../render_info.h"

// TODO: cambiar a level data
void LevelView::update(const std::vector<MineralData>& minerals) {
    for (const MineralData& data : minerals) {
        auto it = ores.find(data.id);
        if (it == ores.end()) {
            ores.emplace(data.id, OreView(data));
        } else {
            it->second.update(data);
        }
    }

    for (auto it = ores.begin(); it != ores.end();) {
        const auto found = std::find_if(minerals.begin(), minerals.end(),
            [id = it->first](const MineralData& data) { return data.id == id; });
        if (found == minerals.end()) {
            it = ores.erase(it);
        } else {
            ++it;
        }
    }
}

void LevelView::draw_background(const RenderInfo&) {
    // TODO: implementar background
}

void LevelView::collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) {
    uint64_t tie = static_cast<uint64_t>(number) << 32;
    for (auto& entry : ores) {
        OreView& ore = entry.second;
        if (const auto depth = ore.depth(ctx)) {
            out.push_back(Drawable{*depth, tie++, &ore});
        }
    }
}

std::optional<ClickTarget> LevelView::pick_target(int world_x, int world_y) const  {
    for (const auto& [id, ore]: ores) {
        if (ore.is_hit(world_x, world_y)) {
            return ClickTarget{TargetType::Ore, id};
        }
    }
    return std::nullopt;
}