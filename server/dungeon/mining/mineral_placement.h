#pragma once

#include <cstdint>
#include <optional>
#include <random>
#include <vector>
#include "position.h"

class DungeonRoom;

class MineralPlacement {
public:
    std::optional<std::vector<Position>> choose(const DungeonRoom& room,uint8_t densityPercent,const std::vector<Position>& reserved,std::mt19937& random) const;
};