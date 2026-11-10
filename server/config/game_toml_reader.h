#pragma once

#include <string>

#include "config/game_config.h"

class GameTomlReader {
public:
    GameConfig read(const std::string& path) const;
};
