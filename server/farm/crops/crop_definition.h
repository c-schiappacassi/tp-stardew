#pragma once

#include <chrono>
#include <cstdint>
#include "protocol_types.h"

struct CropDefinition {
    CropType type;
    std::chrono::milliseconds growthTime;
    std::chrono::milliseconds wateringInterval;
    std::chrono::milliseconds regrowthTime;
    uint32_t toleratedMissedWaterings;
    uint32_t basePrice;
};
