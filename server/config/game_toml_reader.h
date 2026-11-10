#pragma once

#include <string>

#include "config/game_config.h"
#include "farm/loading/farm_toml_reader.h"

class GameTomlReader {
public:
    WorldData read(const std::string& path) const;

    FarmLayout readFarm(const std::string& path, const FarmConfig& config = FarmConfig{}) const;
};
