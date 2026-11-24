#pragma once
#include <memory>
#include <string>
#include <utility>

#include "../../world/hud/ready_to_sleep_view.h"
#include "modal_layer.h"
#include "state.h"

class ReadyToSleepLayer : public ModalLayer {
private:
    std::string player_name;
    ReadyToSleepView view;
    bool open = false;
    bool confirmed = false;
    FocusWish wish = FocusWish::None;

    void confirm(Out& out) {
        if (confirmed) return;
        confirmed = true;
        out.push_back(std::make_shared<DayOkDTO>(player_name));
    }

protected:
    Focus on_key(Key key, Out& out) override {
        if (key == Key::Enter || key == Key::Space) confirm(out);
        return Focus::Keep;
    }

    Focus on_button(std::size_t, Out& out) override {
        confirm(out);
        return Focus::Keep;
    }

public:
    explicit ReadyToSleepLayer(std::string name) : player_name(std::move(name)) {}

    bool visible() const override { return open; }
    FocusWish focus_wish() const override { return wish; }

    bool waiting() const { return open && confirmed; }

    void update(const State& state) override {
        wish = FocusWish::None;
        const bool required = state.day_confirmation_required();
        if (required && !open) {
            open = true;
            confirmed = false;
            wish = FocusWish::Acquire;
        } else if (!required && open) {
            open = false;
            confirmed = false;
            wish = FocusWish::Release;
        }
    }

    void draw(const RenderInfo& ctx) override {
        const auto layout = ReadyToSleepView::layout(ctx, confirmed);
        sync_hitboxes(ctx, layout.buttons);
        view.draw(ctx, layout, confirmed, hovered_buttons());
    }
};