#pragma once
#include <memory>
#include <string>
#include <utility>

#include "../../world/hud/quit_view.h"
#include "../ui_manager.h"
#include "modal_layer.h"

class QuitLayer : public ModalLayer {
private:
    UiManager& ui;
    std::string player_name;
    QuitView view;
    bool confirmed = false;

    void confirm(Out& out) {
        if (confirmed) return;
        confirmed = true;
        out.push_back(std::make_shared<DisconnectDTO>(player_name));
        ui.request_quit();
    }

protected:
    Focus on_key(Key key, Out& out) override {
        if (key == Key::Escape) return Focus::Release;
        if (key == Key::Enter) confirm(out);
        return Focus::Keep;
    }

    Focus on_button(std::size_t index, Out& out) override {
        if (index == 0) {
            confirm(out);
            return Focus::Keep;
        }
        return Focus::Release;
    }

public:
    QuitLayer(UiManager& ui, std::string name) : ui(ui), player_name(std::move(name)) {}

    bool visible() const override { return focused(); }
    void on_focus() override { confirmed = false; }

    void draw(const RenderInfo& ctx) override {
        const auto layout = QuitView::layout(ctx);
        sync_hitboxes(ctx, layout.buttons);
        view.draw(ctx, layout, hovered_buttons());
    }
};