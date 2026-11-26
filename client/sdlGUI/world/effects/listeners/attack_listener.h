#pragma once

#include "position.h"
#include "protocol_types.h"

struct AttackStart {
    EnemyType type;
    Position pos;
};

class AttackListener {
public:
    virtual void on_attack(const AttackStart&) {}
    virtual ~AttackListener() = default;
};
