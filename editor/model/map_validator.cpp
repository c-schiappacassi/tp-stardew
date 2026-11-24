#include "map_validator.h"

#include "common/map/map_rules.h"
#include "map_wriiter.h"

std::vector<std::string> validateMap(const WorldMap& map) { return map_rules::validateMap(toMapData(map)); }
