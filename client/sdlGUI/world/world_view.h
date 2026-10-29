#pragma once
#include "player_view.h"
#pragma once
#include <map>
#include <string>

#include "player_view.h"
#include "hud/hud_view.h"
#include "../render_info.h"
#include "../../state.h"

class WorldView
{
private:
    std::string local_name;
    std::map<std::string, PlayerView> player_views;
    HudView player_hud;

    void update_players_and_list(const std::map<std::string, PlayerData>& players_data) {
        for (const auto& [username, data] : players_data) {
            auto [it, _] = player_views.try_emplace(username, username, data);
            it->second.update(data);
        }

        for (auto it = player_views.begin(); it != player_views.end();) {
            if (players_data.find(it->first) == players_data.end()) it = player_views.erase(it);
            else ++it;
        }
    }
public:
    explicit WorldView(const std::string& local_name){
        this->local_name = local_name;
    };

    void update(const State& state){
        //map.update
        this->update_players_and_list(state.get_players());

        const PlayerData* me = state.find_player(this->local_name);
        this->player_hud.update(*me);
    };

    void draw(const RenderInfo& ctx){
        
        // TODO: las entidades todas tienen depth, update y draw, podria hacer una interfaz. o un entity manager
        for (auto& [username, player] : this->player_views) {
            player.draw(ctx);
        }
        this->player_hud.draw(ctx);
    }
};