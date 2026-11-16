#pragma once
#include "dialog_view.h"

class ReadyToSleepView {
private:
    DialogView dialog;

public:
    using Layout = DialogView::Layout;

    static DialogView::Spec spec(bool waiting) {
        if (waiting) return {{"Waiting for other players..."}, {}, 0};
        return {{"The day is over.", "Ready to go to sleep?"}, {UiSpriteId::OkButton}, 12};
    }

    static Layout layout(const RenderInfo& ctx, bool waiting) {
        return DialogView::layout(ctx, spec(waiting));
    }

    void draw(const RenderInfo& ctx, const Layout& l, bool waiting, const std::vector<bool>& hovered) const {
        dialog.draw(ctx, spec(waiting), l, hovered);
    }
};