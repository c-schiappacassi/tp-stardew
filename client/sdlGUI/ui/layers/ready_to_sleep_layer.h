#pragma once
#include <memory>
#include <string>
#include <utility>

#include "../../world/hud/ready_to_sleep_view.h"
#include "modal_layer.h"
#include "state.h"

class ReadyToSleepLayer : public ModalLayer {
private:
    static constexpr std::size_t BTN_OK = 0;
    static constexpr std::size_t BTN_CANCEL = 1;

    std::string player_name;
    ReadyToSleepView view;
    bool open = false;
    bool confirmed = false;
    bool cancelled = false;
    FocusWish wish = FocusWish::None;
    bool unconscious = false;

    bool showing_wait() const { return confirmed || unconscious; }

    void confirm(Out& out) {
        if (confirmed) return;
        confirmed = true;
        cancelled = false;
        out.push_back(std::make_shared<DayOkDTO>(player_name));
    }

    void cancel(Out& out) {
        if (cancelled) return;
        cancelled = true;
        confirmed = false;
        out.push_back(std::make_shared<DayCancelDTO>(player_name));
    }

protected:
    Focus on_key(Key key, Out& out) override {
        if (unconscious) return Focus::Keep;
        if (key == Key::Enter || key == Key::Space) confirm(out);
        else if (key == Key::Escape) cancel(out);
        return Focus::Keep;
    }

    Focus on_button(std::size_t index, Out& out) override {
        if (unconscious) return Focus::Keep;
        if (index == BTN_OK) confirm(out);
        else if (index == BTN_CANCEL) cancel(out);
        return Focus::Keep;
    }
public:
    explicit ReadyToSleepLayer(std::string name) : player_name(std::move(name)) {}

    bool visible() const override { return open; }
    FocusWish focus_wish() const override { return wish; }

    bool waiting() const { return open && showing_wait(); }
    void update(const State& state) override {
        wish = FocusWish::None;
        const bool required = state.day_confirmation_required();

        const PlayerState s = state.get_local().state;
        const bool resting = s == PlayerState::Sleeping || s == PlayerState::Unconscious;
        unconscious = (s == PlayerState::Unconscious) && !required;
        const bool keep_waiting = confirmed && resting;
        const bool want_open = required || keep_waiting || unconscious;

        if (want_open && !open) {
            open = true;
            confirmed = false;
            cancelled = false;
            wish = unconscious ? FocusWish::None : FocusWish::Acquire;
        } else if (!want_open && open) {
            open = false;
            confirmed = false;
            cancelled = false;
            wish = FocusWish::Release;
        }
    }

    void draw(const RenderInfo& ctx) override {
        const auto layout = ReadyToSleepView::layout(ctx, showing_wait());
        sync_hitboxes(ctx, layout.buttons);
        view.draw(ctx, layout, showing_wait(), hovered_buttons());
    }
};
