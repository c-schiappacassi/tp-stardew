#pragma once
#include <memory>
#include <string>
#include <utility>

#include "../../world/hud/ready_to_sleep_view.h"
#include "../ui_manager.h"
#include "modal_layer.h"
#include "state.h"

class ReadyToSleepLayer : public ModalLayer {
private:
    UiManager& ui;
    std::string player_name;
    ReadyToSleepView view;
    bool open = false;
    bool confirmed = false;

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
    ReadyToSleepLayer(UiManager& ui, std::string name) : ui(ui), player_name(std::move(name)) {}

    bool visible() const override { return open; }

    void update(const State& state) override {
        const bool required = state.day_confirmation_required();
        if (required && !open) {
            open = true;
            confirmed = false;
            ui.focus(*this);
        } else if (!required && open) {
            open = false;
            confirmed = false;
            ui.release(*this);
        }
    }

    void draw(const RenderInfo& ctx) override {
        const auto layout = ReadyToSleepView::layout(ctx, confirmed);
        sync_hitboxes(ctx, layout.buttons);
        view.draw(ctx, layout, confirmed, hovered_buttons());
    }
};