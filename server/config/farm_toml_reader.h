#pragma once

#include "farm/farm_config.h"
#include "farm/loading/farm_layout.h"

#include <toml++/toml.hpp>

class FarmTomlReader {
private:
    const FarmConfig& config;

public:
    explicit FarmTomlReader(const FarmConfig& config);

    FarmLayout read(const toml::table& root) const;
};
