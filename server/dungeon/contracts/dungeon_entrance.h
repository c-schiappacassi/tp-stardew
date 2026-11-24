#pragma once

#include "world/zone_map.h"

#include "protocol_types.h"

struct DungeonEntrance {
    Zone zone = Zone::Farm;
    Position position{0, 0};
    Position returnPosition{0, 0};
    ZoneMap terrain;
};