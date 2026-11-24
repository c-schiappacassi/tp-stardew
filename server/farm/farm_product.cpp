#include "farm_product.h"

std::optional<ItemId> farm_products::cropItem(CropType type) {
    switch (type) {
        case CropType::Strawberry: return ItemId::Strawberry;
        case CropType::Scallion: return ItemId::Scallion;
        case CropType::Potato: return ItemId::Potato;
        case CropType::Pumpkin: return ItemId::Pumpkin;
    }
    return std::nullopt;
}
