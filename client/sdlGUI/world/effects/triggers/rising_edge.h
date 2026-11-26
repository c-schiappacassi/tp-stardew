#pragma once

#include <optional>

class RisingEdge {
    std::optional<bool> last;

public:
    bool update(bool now) {
        const bool fired = last.has_value() && !*last && now;
        last = now;
        return fired;
    }
};
