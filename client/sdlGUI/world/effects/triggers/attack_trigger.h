#pragma once

#include "trigger.h"
#include "rising_edge.h"
#include "../listeners/attack_listener.h"
#include "position.h"
#include "protocol_types.h"

class AttackTrigger : public Trigger<AttackListener> {
    RisingEdge edge;

public:
    void observe(EnemyState state, EnemyType type, const Position& pos) {
        if (!edge.update(state == EnemyState::Attacking)) return;
        const AttackStart start{type, pos};
        notify([&](AttackListener& l) { l.on_attack(start); });
    }
};
