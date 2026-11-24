#pragma once

#include <toml++/toml.hpp>

#include "community/domain/community_config.h"

class CommunityTomlReader {
public:
    CommunityConfig read(const toml::table& root) const;
};
