#pragma once

#include "position.h"
#include "protocol_types.h"

struct FrenzyStart {
    EnemyType type;
    Position pos;
};

class FrenzyListener {
public:
    virtual void on_frenzy(const FrenzyStart&) {}
    virtual ~FrenzyListener() = default;
};
