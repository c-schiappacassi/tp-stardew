#pragma once

#include "config/game_config.h"

#include <toml++/toml.hpp>

class DeathTomlReader {
public:
    DeathConfig read(const toml::table& root) const;
};
