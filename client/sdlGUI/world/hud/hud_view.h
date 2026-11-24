#pragma once
 
#include <set>
 
#include "../../../state.h"
#include "../../render_info.h"
#include "bar_view.h"
#include "hotbar_view.h"
#include "info_view.h"
#include "game_time.h"
 
class HudView {
private:
    HotbarView hot_bar;
    BarView energy_bar{UiSpriteId::EnergyBar, 0};
    BarView health_bar{UiSpriteId::HealthBar, 1};
    InfoView player_informator;
 
public:
    void update(const PlayerDataLocal& player, const uint16_t& time, const std::set<ItemId>& to_alert) {
        hot_bar.update(player);
        energy_bar.update(player.energy, player.maxEnergy);
        health_bar.update(player.health, player.maxHealth);
        player_informator.update(player, time, to_alert);
    }
 
    void draw(const RenderInfo& ctx) {
        hot_bar.draw(ctx);
        energy_bar.draw(ctx);
        health_bar.draw(ctx);
        player_informator.draw(ctx);
    }
};
 