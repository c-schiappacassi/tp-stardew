#pragma once

#include "world/world_location.h"
#include "world/zone_map.h"

struct InteriorLayout {
    InteriorLocation location;
    ZoneMap terrain;
    WorldLocation outside;
    std::vector<Position> extraOutside;
    Position returnPosition{0, 0};
    Position entry{0, 0};
    Position exit{0, 0};
    std::vector<Position> beds;
    BackgroundData background;
};
