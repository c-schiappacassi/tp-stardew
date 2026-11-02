#pragma once

#include "protocol_types.h"
#include "world/zone_map.h"

struct DungeonEntrance {
    Zone zone = Zone::Farm;
    Position position{0, 0};
    Position returnPosition{0, 0};
    ZoneMap terrain;
};