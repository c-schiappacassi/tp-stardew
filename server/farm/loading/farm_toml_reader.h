
#pragma once

#include <string>
#include <toml++/toml.hpp>

#include "farm/farm_config.h"
#include "farm_layout.h"

class FarmTomlReader {
public:
    FarmLayout read(const std::string& path,
                    const FarmConfig& config = FarmConfig{}) const;

    FarmLayout read(const toml::table& root,
                    const FarmConfig& config = FarmConfig{}) const;
};
