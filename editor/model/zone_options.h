#ifndef ZONE_OPTIONS_H
#define ZONE_OPTIONS_H

#include <vector>

#include "catalog.h"

struct ZoneOptions {
    std::vector<TileTextureId> floors;
    std::vector<ElementId> elements;
};

ZoneOptions farmOptions();
ZoneOptions townOptions();
ZoneOptions mineOptions();

#endif  // ZONE_OPTIONS_H