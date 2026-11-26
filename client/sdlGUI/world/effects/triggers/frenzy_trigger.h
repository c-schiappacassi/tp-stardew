#pragma once

#include "trigger.h"
#include "rising_edge.h"
#include "../listeners/frenzy_listener.h"
#include "position.h"
#include "protocol_types.h"

class FrenzyTrigger : public Trigger<FrenzyListener> {
    RisingEdge edge;

public:
    void observe(bool frenzied, EnemyType type, const Position& pos) {
        if (!edge.update(frenzied)) return;
        const FrenzyStart start{type, pos};
        notify([&](FrenzyListener& l) { l.on_frenzy(start); });
    }
};
