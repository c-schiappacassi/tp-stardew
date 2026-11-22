#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct TownPosition {
    uint16_t x;
    uint16_t y;
};

struct TownMapConfig {
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t tileSize = 16;
};

struct TownBuildingConfig {
    std::string id;
    TownPosition position;
    uint16_t width = 0;
    uint16_t height = 0;
    std::vector<TownPosition> door;
};

struct TownNpcConfig {
    std::string id;
    TownPosition position;
};

struct TownConnectionConfig {
    std::string destination;
    TownPosition position;
};

struct TownConfig {
    TownMapConfig map;
    std::vector<TownBuildingConfig> buildings;
    std::vector<TownNpcConfig> npcs;
    std::vector<TownConnectionConfig> connections;
    std::vector<std::vector<bool>> walkableTiles;
};
