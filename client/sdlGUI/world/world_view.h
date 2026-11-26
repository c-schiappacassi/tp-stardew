#pragma once
#include <map>
#include <optional>
#include <string>

#include "../../state.h"
#include "../camera.h"
#include "../sync_views.h"
#include "../render_info.h"
#include "lighting.h"

#include "map_view.h"
#include "player/player_view.h"

class WorldView {
private:
    std::string local_name;
    std::map<std::string, PlayerView> player_views;
    LightMap light_map;
    MapView map;

    void update_players_and_list(const std::map<std::string, PlayerData>& players_data) {
        sync_views_map(player_views, players_data);
    }

    static bool same_zone(const PlayerData& a, const PlayerData& b) {
        if (a.currentZone == Zone::InsideHouse && b.currentZone == Zone::InsideHouse){return a.interior == b.interior; }
        if (a.currentZone == Zone::Dungeon && b.currentZone == Zone::Dungeon){return a.dungeonLevel == b.dungeonLevel; }
        return a.currentZone == b.currentZone;
    }

public:
    explicit WorldView(const std::string& local_name, const ConstMapData& map_data, AssetManager& assets, Audio& audio) : local_name(local_name), map(map_data, assets, audio) {}
    
    std::optional<SDL2pp::Point> world_size() const { return map.world_size(); }

    void update(const State& state) {
        this->update_players_and_list(state.get_players());
        this->map.update(state);
    }

    void draw(const RenderInfo& ctx) {
        const PlayerData& local = ctx.state.get_local();
        const std::map<std::string, PlayerData>&  players = ctx.state.get_players();
        std::vector<Drawable> drawables;
        uint64_t tiebreak = 0;
        std::vector<lighting::Light> lights;

        map.draw_background(ctx);

        for (auto& [name, view]: player_views) {
            if (same_zone(local, players.at(name))) {
                if (const auto d = view.depth(ctx)) { drawables.push_back(Drawable{*d, tiebreak++, &view}); }
            }
        }

        map.collect_drawables(ctx, drawables);

        std::sort(drawables.begin(), drawables.end(), [](const Drawable& a, const Drawable& b) {
            return a.depth != b.depth ? a.depth < b.depth : a.tiebreak < b.tiebreak;
        });

        for (const Drawable& d: drawables) { d.view->draw(ctx); }

        for (auto& [name, view] : player_views) {
            if (same_zone(local, players.at(name))) {
                if (const auto l = view.light(ctx)) lights.push_back(*l);
                if (name != local_name) {
                    view.draw_name(ctx);
                }
            }
        }
        light_map.draw(ctx, map.ambient_light(ctx), lights);
    }
};
