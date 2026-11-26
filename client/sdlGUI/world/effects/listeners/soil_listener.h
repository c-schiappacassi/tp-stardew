#pragma once

#include "position.h"

struct SoilChange {
    Position pos;
};

class SoilListener {
public:
    virtual void on_tilled(const SoilChange&) {}
    virtual ~SoilListener() = default;
};
