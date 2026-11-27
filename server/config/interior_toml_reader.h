#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <toml++/toml.hpp>

#include "world/interiors/interior_layout.h"

class InteriorTomlReader {
private:
    static const toml::array& arrayAt(const toml::node* node, std::string_view field);
    static const toml::array& tupleAt(const toml::node* node, std::size_t size, std::string_view field);
    static int numberAt(const toml::node* node, int maximum, const std::string& field);
    static Position baseAt(const toml::array& tuple, const ZoneMap& terrain, int width, int height, const std::string& field);
    static Position positionAt(const toml::node* node, const ZoneMap& terrain, int width, int height, const std::string& field);
    static InteriorTemplate readTemplate(const toml::table& table, InteriorId id, const std::string& field);

public:
    InteriorTemplates readFile(const std::string& path) const;
    InteriorTemplates read(const toml::table& root) const;
};
