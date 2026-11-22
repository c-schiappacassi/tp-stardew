#include "farm_interactions.h"

#include "chests/interactions/chest_interactions.h"
#include "entities/entity_id_generator.h"
#include "farm/farm_constants.h"
#include "farm/farm_manager.h"
#include "player/player_manager.h"

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

bool FarmInteractions::useShovel(const PlayerManager& players, uint16_t playerId, FarmManager& farm, Direction direction, EntityIdGenerator& entityIds) const {
    const auto player = players.snapshot(playerId);
    const auto position = targeting.findAdjacentPosition(player, direction, farm.getMap(), CollisionBox{entity_dimensions::CROP});
    if (!position) {
        return false;
    }
    for (const auto& [id, decoration]: farm.getDecorations()) {
        if (farm.getMap().sameBaseCell(decoration.getPosition(), *position)) {
            if (!farm.getGroundItems().drop(decoration.getPosition(), decoration.getItem(), 1, Quality::None, entityIds)) {
                return false;
            }
            farm.removeDecoration(id);
            return true;
        }
    }
    return farm.tillSoil(*position);
}

bool FarmInteractions::waterCrop(const PlayerManager& players, uint16_t playerId, FarmManager& farm, Direction direction) const {
    const auto player = players.snapshot(playerId);
    const auto* crop = targeting.findCropToWater(player, farm, direction);
    return crop && farm.waterCrop(crop->getId());
}

bool FarmInteractions::collectProduct(const PlayerManager& players, uint16_t playerId, FarmManager& farm, Direction direction, EntityIdGenerator& entityIds) const {
    const auto player = players.snapshot(playerId);
    const auto target = targeting.findCollectionTarget(player, farm, direction);
    if (!target) {
        return false;
    }
    if (target->source == FarmProductSource::Animal) {
        return farm.dropAnimalProduct(target->id, entityIds);
    }
    const auto& product = target->product;
    if (!farm.getGroundItems().drop(target->position, product.item, product.quantity, product.quality, entityIds)) {
        return false;
    }
    farm.harvestCrop(target->id);
    return true;
}

bool FarmInteractions::placeSelectedItem(PlayerManager& players, uint16_t playerId, FarmManager& farm, EntityIdGenerator& entityIds, Direction direction, ItemId item) const {
    const auto player = players.snapshot(playerId);
    const auto cropType = cropTypeForSeed(item);
    const auto size = cropType ? entity_dimensions::CROP : entity_dimensions::DECORATION;
    const auto position = targeting.findAdjacentPosition(player, direction, farm.getMap(), CollisionBox{size});
    if (!position || entityIds.getNextId() == 0) {
        return false;
    }
    const auto id = entityIds.getNextId();
    bool placed = false;
    if (cropType)
        placed = farm.plant(id, *position, *cropType);
    else
        placed = farm.placeDecoration(id, *position, item, direction);
    if (!placed) {
        return false;
    }
    entityIds.generate();
    return players.apply(playerId, player_action::RemoveItem{player.getSelectedSlot(), 1}).applied;
}

bool FarmInteractions::applySelectedItem(PlayerManager& players, uint16_t playerId, FarmManager& farm, EntityIdGenerator& entityIds, Direction direction) const {
    const auto player = players.snapshot(playerId);
    const auto* selectedItem = player.getSelectedItem();
    if (!selectedItem) {
        return collectProduct(players, playerId, farm, direction, entityIds);
    }
    const auto item = selectedItem->item_id;
    switch (item) {
        case ItemId::WateringCan:
            return waterCrop(players, playerId, farm, direction);
        case ItemId::Shovel:
            return useShovel(players, playerId, farm, direction, entityIds);
        default:
            if (cropTypeForSeed(item) || Decoration::isDecoration(item)) {
                return placeSelectedItem(players, playerId, farm, entityIds, direction, item);
            }
            return collectProduct(players, playerId, farm, direction, entityIds);
    }
}

bool FarmInteractions::interact(
    PlayerManager& players, uint16_t playerId, FarmManager& farm, EntityIdGenerator& entityIds, Direction direction, std::chrono::steady_clock::time_point now) const {
    const auto player = players.snapshot(playerId);
    if (player.getZone() != Zone::Farm || !player.canAct()) {
        return false;
    }
    const auto* selectedItem = player.getSelectedItem();
    if (const auto handled = farm.interactWithChest(players, playerId, direction, entityIds, now))
        return *handled;
    players.apply(playerId, player_action::CloseChest{});
    if (!player.canPerformAction(now))
        return false;
    const bool usesTool = selectedItem && (selectedItem->item_id == ItemId::WateringCan || selectedItem->item_id == ItemId::Shovel);
    if (usesTool && !player.canUseTool(now)) {
        return false;
    }
    if (!applySelectedItem(players, playerId, farm, entityIds, direction)) {
        return false;
    }
    players.apply(playerId, player_action::FaceDirection{direction});
    if (usesTool) {
        return players.apply(playerId, player_action::UseTool{now}).applied;
    }
    return players.apply(playerId, player_action::PerformAction{farm_constants::ACTION_ENERGY, farm_constants::ACTION_INTERVAL, now}).applied;
}
