#pragma once

#include <optional>

#include "trigger.h"
#include "../listeners/zone_listener.h"
#include "protocol_types.h"

class ZoneTrigger : public Trigger<ZoneListener> {
    std::optional<Zone> last;

public:
    void observe(Zone zone) {
        if (last && *last == zone) return;
        const ZoneChange change{last, zone};
        last = zone;
        notify([&](ZoneListener& l) { l.on_enter(change); });
    }
};
