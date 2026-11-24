#pragma once

#include "protocol_types.h"

namespace decoration_items {
bool isDecoration(ItemId item);
FurnitureId furnitureFor(ItemId item);
ItemId itemFor(FurnitureId furniture);
}
