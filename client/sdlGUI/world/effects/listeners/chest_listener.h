#pragma once

#include "position.h"

struct ChestOpening {
    Position pos;
};

class ChestListener {
public:
    virtual void on_opened(const ChestOpening&) {}
    virtual void on_closed(const ChestOpening&) {}
    virtual ~ChestListener() = default;
};
