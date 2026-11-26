#pragma once

#include <optional>
#include "protocol_types.h"

struct ZoneChange {
    std::optional<Zone> previous;
    Zone current;
};

class ZoneListener {
public:
    virtual void on_enter(const ZoneChange&) {}
    virtual ~ZoneListener() = default;
};
