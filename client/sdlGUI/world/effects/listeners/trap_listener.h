#pragma once

#include "position.h"
#include "protocol_types.h"

struct TrapActivation {
    TrapType type;
    Position pos;
};

class TrapListener {
public:
    virtual void on_activated(const TrapActivation&) {}
    virtual ~TrapListener() = default;
};
