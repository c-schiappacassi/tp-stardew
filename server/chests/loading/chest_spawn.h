#pragma once

#include "dtos.h"

struct ChestSpawn {
    Position base;
    std::map<uint8_t, Stack> items;
};
