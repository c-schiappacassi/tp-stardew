#pragma once

#include "town/town_config.h"

#include <string>

#include <toml++/toml.hpp>

class TownTomlReader {
public:
    TownConfig read(const toml::table& root) const;

private:
    static TownPosition readPosition(const toml::table& table,
                                    const std::string& field);
};
