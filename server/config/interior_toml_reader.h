#pragma once

#include <string>
#include <toml++/toml.hpp>

#include "world/interiors/interior_layout.h"

class InteriorTomlReader {
public:
    InteriorTemplates readFile(const std::string& path) const;
    InteriorTemplates read(const toml::table& root) const;
};
