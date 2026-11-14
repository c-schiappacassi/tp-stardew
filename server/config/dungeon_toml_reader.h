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
    static Position readCell(const toml::node* node,
                             const std::string& field,
                             int64_t maxX,
                             int64_t maxY);
    static Position readBasePosition(const toml::node* node,
                                     const std::string& field,
                                     const ZoneMap& terrain,
                                     int64_t width,
                                     int64_t height);
};
