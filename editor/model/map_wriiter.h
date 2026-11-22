#ifndef MAP_WRIITER_H
#define MAP_WRIITER_H

#include <string>

#include "world_map.h"

bool writeMap(const WorldMap& map, const std::string& path);

#endif  // MAP_WRIITER_H