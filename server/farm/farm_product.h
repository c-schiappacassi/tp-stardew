#pragma once

#include <optional>
#include "farm/farm_types.h"

namespace farm_products {
std::optional<ItemId> cropItem(CropType type);
}
