#pragma once

#include <vector>

#include "zone_map.h"

struct WorldMap {
    ZoneMap farm;
    ZoneMap town;
    std::vector<ZoneMap> mineLevels;

    WorldMap();
};