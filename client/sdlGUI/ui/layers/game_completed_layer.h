#pragma once
#include <memory>
#include <string>
#include <utility>

#include "../../world/hud/game_completed_view.h"
#include "../input_event.h"
#include "../ui_layer.h"
#include "state.h"

class GameCompletedLayer : public UiLayer {
private:
    std::string player_name;
    GameCompletedView view;
    bool completed = false;
    bool exit_requested = false;
    FocusWish wish = FocusWish::None;

    void request_exit(Out& out) {
        if (exit_requested) return;
        exit_requested = true;
        out.push_back(std::make_shared<DisconnectDTO>(player_name));
    }

public:
    explicit GameCompletedLayer(std::string name) : player_name(std::move(name)) {}

    bool visible() const override { return view.visible(); }
    bool wants_quit() const override { return exit_requested; }
    FocusWish focus_wish() const override { return wish; }

    void update(const State& state) override {
        wish = FocusWish::None;
        const bool now = state.game_completed();
        if (now && !completed) {
            wish = FocusWish::Acquire;
        } else if (!now && completed) {
            wish = FocusWish::Release;
        }
        completed = now;
        view.update(completed);
    }

    Focus handle(const InputEvent& e, Out& out) override {
        if (e.type == InputEvent::Type::KeyDown && !e.repeat && view.accepts_input() &&
            (e.key == Key::Enter || e.key == Key::Escape)) {
            request_exit(out);
        }
        return Focus::Keep;
    }

    void draw(const RenderInfo& ctx) override { view.draw(ctx); }
};
