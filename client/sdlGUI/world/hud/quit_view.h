#pragma once
#include "dialog_view.h"

class QuitView {
private:
    inline static const std::string QUIT    = "Quit the game?";
    DialogView dialog;

public:
    using Layout = DialogView::Layout;

    static DialogView::Spec spec() {
        return {{QUIT}, {UiSpriteId::OkButton, UiSpriteId::CancelButton}, 14};
    }

    static Layout layout(const RenderInfo& ctx) { return DialogView::layout(ctx, spec()); }

    void draw(const RenderInfo& ctx, const Layout& l, const std::vector<bool>& hovered) const {
        dialog.draw(ctx, spec(), l, hovered);
    }
};