#pragma once

#include <cstdint>
#include <functional>
#include <optional>

#include "protocol_types.h"

struct ClickTarget {
    TargetType type;
    uint32_t id;
};

using TargetPicker = std::function<std::optional<ClickTarget>(int screen_x, int screen_y)>;