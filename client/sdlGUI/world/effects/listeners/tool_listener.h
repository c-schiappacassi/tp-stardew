#pragma once

#include "position.h"
#include "protocol_types.h"

struct ToolUse {
    ItemId tool;
    Direction dir;
    Position pos;
};

class ToolListener {
public:
    virtual void on_tool_use(const ToolUse&) {}
    virtual ~ToolListener() = default;
};
