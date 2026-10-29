#pragma once
#include <map>
#include <string>

#include "../player_view.h"
#include "../../render_info.h"
#include "../../../state.h"
#include "hotbar_view.h"

class HudView
{
private:
   HotbarView hot_bar;

   // TODO:
   // InfoView info; // no le puse clock porque tambien muestra la cantidad de plata, como en el juego
   // Bar heath_bar;
   // Bar energy_bar;
public:
    explicit HudView() : hot_bar(HotbarView()) {};

    void update(const PlayerData& player_data){
        hot_bar.update(player_data);
    };

    void draw(const RenderInfo& ctx){
        hot_bar.draw(ctx);
    }
};