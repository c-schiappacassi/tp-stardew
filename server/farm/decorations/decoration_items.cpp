#include "decoration_items.h"

bool decoration_items::isDecoration(ItemId item) {
    return item >= ItemId::Chair && item <= ItemId::BrownArmCouch && item != ItemId::Bed && item != ItemId::BedSheet;
}

FurnitureId decoration_items::furnitureFor(ItemId item) {
    return static_cast<FurnitureId>(static_cast<uint16_t>(item) - static_cast<uint16_t>(ItemId::Chair));
}

ItemId decoration_items::itemFor(FurnitureId furniture) {
    return static_cast<ItemId>(static_cast<uint16_t>(ItemId::Chair) + static_cast<uint16_t>(furniture));
}
