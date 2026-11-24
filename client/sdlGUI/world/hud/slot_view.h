#pragma once
#include <cstdint>
#include <optional>
#include <string>

#include <SDL2pp/SDL2pp.hh>

#include "../../render_info.h"

#include "dtos.h"
#include "protocol_types.h"

namespace slot_layout {
    constexpr int SLOT_PX = 64;
    constexpr int PANEL_CORNER = 16;
}

class SlotView {
private:
    std::optional<Stack> stack;
    bool selected = false;
    std::string label;

public:
    SlotView() = default;

    void set_stack(const std::optional<Stack>& new_stack) { stack = new_stack; }
    void set_selected(bool value) { selected = value; }
    void set_label(std::string text) { label = std::move(text); }

    const std::optional<Stack>& get_stack() const { return stack; }
    bool empty() const { return !stack.has_value(); }

    void draw(const RenderInfo& ctx, const SDL2pp::Rect& rect, int u) const;
};
