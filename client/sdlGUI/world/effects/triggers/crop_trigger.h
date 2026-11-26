#pragma once

#include <optional>

#include "trigger.h"
#include "../listeners/crop_listener.h"
#include "position.h"
#include "protocol_types.h"

class CropTrigger : public Trigger<CropListener> {
    std::optional<bool> last_watered;

public:
    void observe(bool watered, CropType type, const Position& pos) {
        if (last_watered && !*last_watered && watered) {
            const CropWatering watering{type, pos};
            notify([&](CropListener& l) { l.on_watered(watering); });
        }
        last_watered = watered;
    }
};
