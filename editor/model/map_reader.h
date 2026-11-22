#ifndef MAP_READER_H
#define MAP_READER_H

#include <string>

#include "world_map.h"

WorldMap readMap(const std::string& path);

#endif  // MAP_READER_H
