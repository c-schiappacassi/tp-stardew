#include "farm_interactions.h"

#include "entities/entity_id_generator.h"
#include "farm/farm_constants.h"
#include "farm/farm_manager.h"
#include "player/player.h"

namespace {
std::optional<CropType> cropTypeForSeed(ItemId item) {
    switch (item) {
        case ItemId::StrawberrySeed:
            return CropType::Strawberry;
        case ItemId::ScallionSeed:
            return CropType::Scallion;
        case ItemId::PotatoSeed:
            return CropType::Potato;
        case ItemId::PumpkinSeed:
            return CropType::Pumpkin;
        default:
            return std::nullopt;
    }
}
}  // namespace

bool FarmInteractions::useShovel(Player& player, FarmManager& farm, Direction direction,
    EntityIdGenerator& entityIds) const {
    const auto position = targeting.findAdjacentPosition(player, direction, farm.getMap(),
        CollisionBox{entity_dimensions::CROP});
    if (!position) {
        return false;
    }
    for (const auto& [id, decoration] : farm.getDecorations()) {
        if (farm.getMap().sameBaseCell(decoration.getPosition(), *position)) {
            if (!farm.getGroundItems().drop(decoration.getPosition(), decoration.getItem(),
                    1, Quality::None, entityIds)) {
                return false;
            }
            farm.removeDecoration(id);
            return true;
        }
    }
    return farm.tillSoil(*position);
}

bool FarmInteractions::waterCrop(Player& player, FarmManager& farm, Direction direction) const {
    const auto* crop = targeting.findCropToWater(player, farm, direction);
    return crop && farm.waterCrop(crop->getId());
}

bool FarmInteractions::collectProduct(Player& player, FarmManager& farm, Direction direction, EntityIdGenerator& entityIds) const {
    const auto target = targeting.findCollectionTarget(player, farm, direction);
    if (!target) {
        return false;
    }
    if (target->source == FarmProductSource::Animal) {
        return farm.dropAnimalProduct(target->id, entityIds);
    }
    const auto& product = target->product;
    if (!farm.getGroundItems().drop(target->position, product.item, product.quantity,
            product.quality, entityIds)) {
        return false;
    }
    farm.harvestCrop(target->id);
    return true;
}

bool FarmInteractions::placeSelectedItem(Player& player, FarmManager& farm,
    EntityIdGenerator& entityIds, Direction direction, ItemId item) const {
    const auto cropType = cropTypeForSeed(item);
    const auto size = cropType ? entity_dimensions::CROP : entity_dimensions::DECORATION;
    const auto position = targeting.findAdjacentPosition(player, direction, farm.getMap(),
        CollisionBox{size});
    if (!position || entityIds.getNextId() == 0) {
        return false;
    }
    const auto id = entityIds.getNextId();
    const bool placed = cropType ? farm.plant(id, *position, *cropType) :
        farm.placeDecoration(id, *position, item, direction);
    if (!placed) {
        return false;
    }
    entityIds.generate();
    return player.removeItem(player.getSelectedSlot(), 1);
}

bool FarmInteractions::applySelectedItem(Player& player, FarmManager& farm,
    EntityIdGenerator& entityIds, Direction direction) const {
    const auto* selectedItem = player.getSelectedItem();
    if (!selectedItem) {
        return collectProduct(player, farm, direction, entityIds);
    }
    const auto item = selectedItem->item_id;
    switch (item) {
        case ItemId::WateringCan:
            return waterCrop(player, farm, direction);
        case ItemId::Shovel:
            return useShovel(player, farm, direction, entityIds);
        default:
            if (cropTypeForSeed(item) || Decoration::isDecoration(item)) {
                return placeSelectedItem(player, farm, entityIds, direction, item);
            }
            return collectProduct(player, farm, direction, entityIds);
    }
}

bool FarmInteractions::interact(Player& player, FarmManager& farm, EntityIdGenerator& entityIds,
    Direction direction, std::chrono::steady_clock::time_point now) const {
    if (player.getZone() != Zone::Farm || !player.canPerformAction(now)) {
        return false;
    }
    const auto* selectedItem = player.getSelectedItem();
    const bool usesTool = selectedItem && (selectedItem->item_id == ItemId::WateringCan ||
        selectedItem->item_id == ItemId::Shovel);
    if (usesTool && !player.canUseTool(now)) {
        return false;
    }
    if (!applySelectedItem(player, farm, entityIds, direction)) {
        return false;
    }
    player.faceDirection(direction);
    if (usesTool) {
        return player.useTool(now);
    }
    return player.performAction(farm_constants::ACTION_ENERGY,
        farm_constants::ACTION_INTERVAL, now);
}
