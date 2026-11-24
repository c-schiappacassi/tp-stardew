#pragma once

#include <string>
#include <vector>

#include "common/map/map_data.h"
#include "world_map.h"

std::vector<bool> bakedWalkability(const ZoneMap& zone);

MapData toMapData(const WorldMap& map);

bool writeMap(const WorldMap& map, const std::string& path);