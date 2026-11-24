#pragma once
#include <algorithm>
#include <cmath>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../../render_info.h"
#include "ready_to_sleep_layer.h"
#include "../../world/hud/night_sky_view.h"
#include "../ui_layer.h"
#include "state.h"

class NightSkyLayer : public UiLayer {
private:
    const ReadyToSleepLayer* sleep_layer = nullptr;
    NightSkyView view;
    bool active = false;

    static bool out_for_the_day(PlayerState s) {
        return s == PlayerState::Unconscious || s == PlayerState::Sleeping;
    }

public:
    void watch(const ReadyToSleepLayer& layer) { sleep_layer = &layer; }

    bool visible() const override { return view.visible(); }

    void update(const State& state) override {
        const bool waiting = sleep_layer != nullptr && sleep_layer->waiting();
        active = out_for_the_day(state.get_local().state) || waiting;

        view.update(active);
    }

    void draw(const RenderInfo& ctx) override {
        view.draw(ctx);
    }
};