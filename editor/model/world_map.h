#ifndef WORLD_MAP_H
#define WORLD_MAP_H

#include <vector>

#include "zone_map.h"

struct WorldMap {
    ZoneMap farm;
    ZoneMap town;
    std::vector<ZoneMap> mineLevels;

    WorldMap();
};

#endif  // WORLD_MAP_H