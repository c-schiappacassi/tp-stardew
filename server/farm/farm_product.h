#pragma once

#include <cstdint>
#include <optional>

#include "protocol_types.h"

struct FarmProduct {
    ItemId item;
    uint32_t quantity;
    Quality quality;
};

namespace farm_products {
std::optional<ItemId> cropItem(CropType type);
}
