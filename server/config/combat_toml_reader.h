#pragma once

#include <toml++/toml.hpp>

#include "dungeon/dungeon_config.h"
#include "player/player_config.h"

class CombatTomlReader {
public:
    void read(const toml::table& root, DungeonConfig& dungeon, PlayerConfig& player) const;
};
