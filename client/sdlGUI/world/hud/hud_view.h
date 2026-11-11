#pragma once
#include <map>
#include <string>

#include "../../../state.h"
#include "../../render_info.h"
#include "../player_view.h"

#include "hotbar_view.h"
#include "bar_view.h"

class HudView {
private:
    HotbarView hot_bar;
    BarView energy_bar;
    BarView health_bar;

public:
    explicit HudView(): hot_bar(HotbarView()), energy_bar(BarView('E', 0)), health_bar(BarView('H', 1)){};

    void update(const PlayerDataLocal& player_data) {
        hot_bar.update(player_data);
        energy_bar.update(player_data.energy, player_data.maxEnergy);
        health_bar.update(player_data.health, player_data.maxHealth);
    }

    void draw(const RenderInfo& ctx) {
        hot_bar.draw(ctx);
        energy_bar.draw(ctx);
        health_bar.draw(ctx);
    }
};