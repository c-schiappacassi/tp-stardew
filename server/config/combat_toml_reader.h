#pragma once

#include <cstdint>
#include <string_view>
#include <toml++/toml.hpp>

#include "dungeon/dungeon_config.h"
#include "player/player_config.h"

class CombatTomlReader {
private:
    static uint32_t readNumber(const toml::node_view<const toml::node>& section, std::string_view name,
                                uint32_t current, uint32_t maximum);
    static void number(const toml::node_view<const toml::node>& section, std::string_view name,
                        uint8_t& target);
    static void number(const toml::node_view<const toml::node>& section, std::string_view name,
                        uint16_t& target);
    static void number(const toml::node_view<const toml::node>& section, std::string_view name,
                        uint32_t& target);

public:
    void read(const toml::table& root, DungeonConfig& dungeon, PlayerConfig& player) const;
};
