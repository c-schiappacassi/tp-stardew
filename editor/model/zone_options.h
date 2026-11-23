#pragma once

#include <vector>

#include "catalog.h"

struct ZoneOptions {
    std::vector<TileTextureId> floors;
    std::vector<ElementId> elements;
};

ZoneOptions farmOptions();
ZoneOptions townOptions();
ZoneOptions mineOptions();