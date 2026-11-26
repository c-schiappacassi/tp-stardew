#pragma once

#include "trigger.h"
#include "rising_edge.h"
#include "../listeners/chest_listener.h"
#include "position.h"

class ChestTrigger : public Trigger<ChestListener> {
    RisingEdge opening;
    RisingEdge closing;

public:
    void observe(bool open, const Position& pos) {
        const ChestOpening event{pos};
        if (opening.update(open)) notify([&](ChestListener& l) { l.on_opened(event); });
        if (closing.update(!open)) notify([&](ChestListener& l) { l.on_closed(event); });
    }
};
