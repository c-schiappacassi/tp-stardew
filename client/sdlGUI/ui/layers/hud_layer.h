#pragma once

#include "../../render_info.h"
#include "../../world/hud/hud_view.h"
#include "state.h"
#include "../ui_layer.h"

class HudLayer : public UiLayer {
private:
    HudView hud;

public:
    void update(const State& state) override { hud.update(state.get_local(), state.get_local_time(), state.get_alerts() ); }

    void draw(const RenderInfo& ctx) override { hud.draw(ctx); }
};
