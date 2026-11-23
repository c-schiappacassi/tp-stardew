#pragma once

#include <string>
#include <vector>

#include "common/map/map_data.h"
#include "common/map/map_shapes.h"

namespace map_rules {

inline constexpr int MIN_FARM_HOUSES = 1;
inline constexpr int MAX_FARM_HOUSES = 4;
inline constexpr int MINE_LEVEL_COUNT = 10;
inline constexpr int PASSAGE_HEIGHT = 3;

bool isFarmBuilding(int id);
bool isTownBuilding(int id);

std::vector<TilePosition> passageTiles(const TilePosition& door);

std::vector<std::string> validateMap(const MapData& map);

std::string joinErrors(const std::vector<std::string>& errors);

}