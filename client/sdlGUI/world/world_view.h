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
    explicit WorldView(const std::string& local_name): local_name(local_name) {}

    void update(const State& state) {
        this->update_players_and_list(state.get_players());
        this->player_hud.update(state.get_local());
        this->map.update(state);
    }

    void draw(const RenderInfo& ctx) {
        map.draw_background(ctx);

        // TODO:

        // map.draw_backgroud(ctx) // le va a pedir a la zona actual que se dibuje

        // meter drawables de los players en vista en un vec

        // map.get_drawables(ctx,vec) // le va pedir a la zona actual que le de los drawables se se vean

        // sort de los drawables junto con los players

        // .draw a lo ordenado a los elem del vec

        std::vector<Drawable> drawables;
        uint64_t tiebreak = 0;

        for (auto& [name, view]: player_views) {
            if (const auto d = view.depth(ctx)) {  // nullopt = fuera de camara
                drawables.push_back(Drawable{*d, tiebreak++, &view});
            }
        }

        map.collect_drawables(ctx, drawables);  // minerales, animales, etc. (por ahora vacio)

        std::sort(drawables.begin(), drawables.end(), [](const Drawable& a, const Drawable& b) {
            return a.depth != b.depth ? a.depth < b.depth : a.tiebreak < b.tiebreak;
        });

        for (const Drawable& d: drawables) {
            d.view->draw(ctx);
        }

        
        this->player_hud.draw(ctx);  // siempre arriba de todo asi que ultimo
    }
};
