#pragma once

#include <set>
#include <utility>
#include <vector>

#include "trigger.h"
#include "../listeners/soil_listener.h"
#include "position.h"

class SoilTrigger : public Trigger<SoilListener> {
    std::set<std::pair<int, int>> known;
    bool primed = false;

public:
    void observe(const std::vector<Position>& tilled) {
        bool changed = !primed || tilled.size() != known.size();

        for (const Position& p : tilled) {
            if (known.count({p.getX(), p.getY()}) != 0) continue;
            changed = true;
            if (primed) {
                const SoilChange change{p};
                notify([&](SoilListener& l) { l.on_tilled(change); });
            }
        }

        if (!changed) return;
        known.clear();
        for (const Position& p : tilled) known.insert({p.getX(), p.getY()});
        primed = true;
    }
};
