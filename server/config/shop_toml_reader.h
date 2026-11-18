#pragma once

#include "config/game_config.h"

#include <string>

#include <toml++/toml.hpp>

class ShopTomlReader {
public:
    RobinShopConfig read(const toml::table& root) const;
    MarketConfig readMarket(const toml::table& root) const;

private:
    static ItemId itemFromId(uint16_t id, const std::string& field);
};
