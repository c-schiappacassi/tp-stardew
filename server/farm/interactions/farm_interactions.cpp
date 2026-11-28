#include "farm_interactions.h"

#include "chests/chest_manager.h"
#include "entities/entity_id_generator.h"
#include "farm/animals/animal_manager.h"
#include "farm/crops/crop_manager.h"
#include "farm/decorations/decoration_manager.h"
#include "farm/farm_constants.h"
#include "farm/spatial/farm_terrain.h"
#include "items/ground_items.h"
#include "player/player_manager.h"

std::optional<CropType> FarmInteractions::cropTypeForSeed(ItemId item) {
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

FarmInteractions::FarmInteractions(FarmTerrain& terrain, CropManager& crops, AnimalManager& animals, DecorationManager& decorations,
    ChestManager& chests, GroundItems& groundItems):
    terrain(terrain), crops(crops), animals(animals), decorations(decorations), chests(chests), groundItems(groundItems) {}

bool FarmInteractions::collectNearbyItems(PlayerManager& players, uint16_t playerId) const {
    const auto player = players.snapshot(playerId);
    if (!terrain.isInitialized() || player.getZone() != Zone::Farm)
        return false;
    return groundItems.collect(players, playerId, [&](const Position& position) { return targeting.hasClearPath(player.getPosition(), position, terrain.getMap()); });
}

bool FarmInteractions::useShovel(const PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& entityIds) const {
    const auto player = players.snapshot(playerId);
    const auto position = targeting.findAdjacentPosition(player, direction, terrain.getMap(), CollisionBox{entity_dimensions::CROP});
    if (!position) {
        return false;
    }
    const auto tile = CollisionBox{entity_dimensions::DECORATION}.at(*position);
    for (const auto& [id, decoration]: decorations.getDecorations()) {
        if (decoration.getBounds().intersects(tile)) {
            if (!groundItems.drop(decoration.getPosition(), decoration.getItem(), 1, Quality::None, entityIds)) {
                return false;
            }
            decorations.remove(id, terrain.getSpace());
            return true;
        }
    }
    return crops.tillSoil(*position, terrain);
}

bool FarmInteractions::waterCrop(const PlayerManager& players, uint16_t playerId, Direction direction) const {
    const auto player = players.snapshot(playerId);
    const auto* crop = targeting.findCropToWater(player, terrain, crops, direction);
    return crop && crops.water(crop->getId());
}

bool FarmInteractions::collectProduct(const PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& entityIds) const {
    const auto player = players.snapshot(playerId);
    const auto target = targeting.findCollectionTarget(player, terrain, crops, animals, direction);
    if (!target) {
        return false;
    }
    if (target->source == FarmProductSource::Animal) {
        return animals.dropProduction(target->id, groundItems, entityIds);
    }
    const auto& product = target->product;
    if (!groundItems.drop(target->position, product.item, product.quantity, product.quality, entityIds)) {
        return false;
    }
    crops.harvest(target->id, terrain.getSpace());
    return true;
}

bool FarmInteractions::placeSelectedItem(PlayerManager& players, uint16_t playerId, EntityIdGenerator& entityIds, Direction direction, ItemId item) const {
    const auto player = players.snapshot(playerId);
    const auto cropType = cropTypeForSeed(item);
    const CollisionBox box = cropType ? CollisionBox{entity_dimensions::CROP} : Decoration::collisionBoxFor(item, direction);
    const auto position = targeting.findAdjacentPosition(player, direction, terrain.getMap(), box);
    if (!position || entityIds.getNextId() == 0) {
        return false;
    }
    const auto id = entityIds.getNextId();
    bool placed = false;
    if (cropType)
        placed = crops.plant(id, *position, *cropType, terrain.getSpace());
    else
        placed = decorations.place(id, *position, item, terrain.getSpace(), direction);
    if (!placed) {
        return false;
    }
    entityIds.generate();
    return players.apply(playerId, player_action::RemoveItem{player.getSelectedSlot(), 1}).applied;
}

bool FarmInteractions::applySelectedItem(PlayerManager& players, uint16_t playerId, EntityIdGenerator& entityIds, Direction direction) const {
    const auto player = players.snapshot(playerId);
    const auto* selectedItem = player.getSelectedItem();
    if (!selectedItem) {
        return collectProduct(players, playerId, direction, entityIds);
    }
    const auto item = selectedItem->item_id;
    switch (item) {
        case ItemId::WateringCan:
            return waterCrop(players, playerId, direction);
        case ItemId::Shovel:
            return useShovel(players, playerId, direction, entityIds);
        default:
            if (cropTypeForSeed(item) || Decoration::isDecoration(item)) {
                return placeSelectedItem(players, playerId, entityIds, direction, item);
            }
            return collectProduct(players, playerId, direction, entityIds);
    }
}

bool FarmInteractions::interact(
    PlayerManager& players, uint16_t playerId, EntityIdGenerator& entityIds, Direction direction, std::chrono::steady_clock::time_point now) const {
    const auto player = players.snapshot(playerId);
    if (player.getZone() != Zone::Farm || !player.canAct()) {
        return false;
    }
    const auto* selectedItem = player.getSelectedItem();
    if (const auto handled = chests.interact(players, playerId, terrain.getSpace(), terrain.getMap(), groundItems, direction, entityIds, now))
        return *handled;
    players.apply(playerId, player_action::CloseChest{});
    if (!player.canPerformAction(now))
        return false;
    const bool usesTool = selectedItem && (selectedItem->item_id == ItemId::WateringCan || selectedItem->item_id == ItemId::Shovel);
    if (usesTool && !player.canUseTool(now)) {
        return false;
    }
    if (!applySelectedItem(players, playerId, entityIds, direction)) {
        return false;
    }
    players.apply(playerId, player_action::FaceDirection{direction});
    if (usesTool) {
        return players.apply(playerId, player_action::UseTool{now}).applied;
    }
    return players.apply(playerId, player_action::PerformAction{farm_constants::ACTION_ENERGY, farm_constants::ACTION_INTERVAL, now}).applied;
}
