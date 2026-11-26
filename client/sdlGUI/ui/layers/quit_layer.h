#pragma once
#include <memory>
#include <string>
#include <utility>

#include "../../world/hud/dialog_view.h"
#include "modal_layer.h"

class QuitLayer : public ModalLayer {
private:
    inline static const DialogView::Spec DIALOG{{"Quit the game?"}, {UiSpriteId::OkButton, UiSpriteId::CancelButton}, 14};

    std::string player_name;
    bool confirmed = false;

    void confirm(Out& out) {
        if (confirmed) return;
        confirmed = true;
        out.push_back(std::make_shared<DisconnectDTO>(player_name));
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
    explicit QuitLayer(std::string name) : player_name(std::move(name)) {}

    bool visible() const override { return focused(); }
    bool wants_quit() const override { return confirmed; }
    void on_focus() override { confirmed = false; }

    void draw(const RenderInfo& ctx) override {
        hits.begin(ctx);
        DialogView::draw(ctx, DIALOG, hits);
    }
};