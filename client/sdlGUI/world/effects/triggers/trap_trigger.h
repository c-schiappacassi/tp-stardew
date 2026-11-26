#pragma once

#include "trigger.h"
#include "rising_edge.h"
#include "../listeners/trap_listener.h"
#include "position.h"
#include "protocol_types.h"

class TrapTrigger : public Trigger<TrapListener> {
    RisingEdge edge;

public:
    void observe(TrapState state, TrapType type, const Position& pos) {
        if (!edge.update(state == TrapState::Active)) return;
        const TrapActivation activation{type, pos};
        notify([&](TrapListener& l) { l.on_activated(activation); });
    }
};
