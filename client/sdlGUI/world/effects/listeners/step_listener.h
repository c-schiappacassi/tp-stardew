#pragma once

#include "position.h"
#include "protocol_types.h"

struct Step {
    Position pos;
    Zone zone;
};

class StepListener {
public:
    virtual void on_step(const Step&) {}
    virtual ~StepListener() = default;
};
