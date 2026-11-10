#pragma once

#include <string>

#include "config/game_config.h"

class GameTomlReader {
public:
    WorldData read(const std::string& path) const;
};
