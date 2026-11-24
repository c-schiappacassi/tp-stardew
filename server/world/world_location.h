#pragma once

#include "dtos.h"

struct WorldLocation {
    Zone zone = Zone::Farm;
    uint8_t level = 0;
    std::optional<InteriorLocation> interior;
    Position position{0, 0};
};

struct ZonePortal {
    WorldLocation source;
    WorldLocation destination;
};
