#pragma once

#include <cstdint>

#include "protocol_types.h"

struct FarmProduct {
    ItemId item;
    uint32_t quantity;
    Quality quality;
};
