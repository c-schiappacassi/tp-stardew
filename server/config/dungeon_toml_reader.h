#pragma once

#include "dungeon/dungeon_config.h"

#include <string>

#include <toml++/toml.hpp>

class DungeonTomlReader {
public:
    DungeonConfig read(const toml::table& root) const;

private:
    static ResourceType resourceTypeFromName(const std::string& name,
                                             const std::string& field);
    static ItemId rewardItemFromName(const std::string& name,
                                     const std::string& field);
    static Quality qualityFromName(const std::string& name,
                                   const std::string& field);
};
