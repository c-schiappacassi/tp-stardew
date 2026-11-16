#pragma once

#include "../../../state.h"
#include "../../render_info.h"
#include "bar_view.h"
#include "hotbar_view.h"

class HudView {
private:
    HotbarView hot_bar;
    BarView energy_bar{UiSpriteId::EnergyBar, 0};
    BarView health_bar{UiSpriteId::HealthBar, 1};

public:
    void update(const PlayerDataLocal& player) {
        hot_bar.update(player);
        energy_bar.update(player.energy, player.maxEnergy);
        health_bar.update(player.health, player.maxHealth);
    }

    void draw(const RenderInfo& ctx) {
        hot_bar.draw(ctx);
        energy_bar.draw(ctx);
        health_bar.draw(ctx);
    }
};