#pragma once

#include "../../world/hud/minimap_view.h"
#include "../ui_layer.h"
#include "state.h"

class MinimapLayer : public UiLayer {
private:
    MinimapView view;

public:
    bool visible() const override { return focused(); }

    void update(const State& state) override { view.update(state.get_players()); }

    Focus handle(const InputEvent& e, Out&) override {
        switch (e.type) {
            case InputEvent::Type::KeyDown:
                if (!e.repeat && (e.key == Key::M || e.key == Key::Escape)) {
                    return Focus::Release;
                }
                break;
            case InputEvent::Type::MouseDown: return Focus::Release;
            default: break;
        }
        return Focus::Keep;
    }

    void draw(const RenderInfo& ctx) override { view.draw(ctx); }
};