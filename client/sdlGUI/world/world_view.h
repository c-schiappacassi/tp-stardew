#pragma once
#include <map>
#include <optional>
#include <string>

#include "../../state.h"
#include "../camera.h"
#include "../sync_views.h"
#include "../render_info.h"
#include "hud/hud_view.h"

#include "map_view.h"
#include "player_view.h"

class WorldView {
private:
    std::string local_name;
    std::map<std::string, PlayerView> player_views;
    HudView player_hud;
    MapView map;

    void update_players_and_list(const std::map<std::string, PlayerData>& players_data) {
        sync_views_map(player_views, players_data);
    }

public:
    explicit WorldView(const std::string& local_name, const ConstMapData& map_data, AssetManager& assets) : local_name(local_name), map(map_data, assets) {}
    
    void update(const State& state) {
        this->update_players_and_list(state.get_players());
        this->player_hud.update(state.get_local());
        this->map.update(state);
    }

    void draw(const RenderInfo& ctx) {
        map.draw_background(ctx);

        std::vector<Drawable> drawables;
        uint64_t tiebreak = 0;

        for (auto& [name, view]: player_views) {
            if (const auto d = view.depth(ctx)) {
                drawables.push_back(Drawable{*d, tiebreak++, &view});
            }
        }

        map.collect_drawables(ctx, drawables);

        std::sort(drawables.begin(), drawables.end(), [](const Drawable& a, const Drawable& b) {
            return a.depth != b.depth ? a.depth < b.depth : a.tiebreak < b.tiebreak;
        });

        for (const Drawable& d: drawables) {
            d.view->draw(ctx);
        }

        this->player_hud.draw(ctx);
    }
};
