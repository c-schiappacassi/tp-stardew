#pragma once
#include "dialog_view.h"

class ReadyToSleepView {
private:
    inline static const std::string WAIT    = "Waiting for other players...";
    inline static const std::string DAY_OVER = "The day is over.";
    inline static const std::string SLEEP     = "Ready to go to sleep?";
    DialogView dialog;

public:
    using Layout = DialogView::Layout;

    static DialogView::Spec spec(bool waiting) {
        if (waiting) return {{WAIT}, {}, 0};
        return {{DAY_OVER, SLEEP}, {UiSpriteId::OkButton, UiSpriteId::CancelButton}, 12};
    }

    static Layout layout(const RenderInfo& ctx, bool waiting) {
        return DialogView::layout(ctx, spec(waiting));
    }

    void draw(const RenderInfo& ctx, const Layout& l, bool waiting, const std::vector<bool>& hovered) const {
        dialog.draw(ctx, spec(waiting), l, hovered);
    }
};