#pragma once

#include <optional>

#include "trigger.h"
#include "../listeners/tool_listener.h"
#include "position.h"
#include "protocol_types.h"

class ToolTrigger : public Trigger<ToolListener> {
    bool was_using = false;

public:
    void observe(PlayerState state, const std::optional<ItemId>& held, Direction dir, const Position& pos) {
        const bool using_now = state == PlayerState::UsingTool && held.has_value();
        if (using_now && !was_using) {
            const ToolUse use{*held, dir, pos};
            notify([&](ToolListener& l) { l.on_tool_use(use); });
        }
        was_using = using_now;
    }
};
