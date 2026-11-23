#pragma once

#include <string>

#include "common/map/map_data.h"
#include "world_map.h"

WorldMap toWorldMap(const MapData& data);


WorldMap readMap(const std::string& path);
