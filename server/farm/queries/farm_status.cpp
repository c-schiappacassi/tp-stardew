#include "farm_status.h"

#include "chests/chest_manager.h"
#include "farm/animals/animal_manager.h"
#include "farm/crops/crop_manager.h"
#include "farm/decorations/decoration_manager.h"
#include "farm/spatial/farm_terrain.h"
#include "items/ground_items.h"

FarmData FarmStatus::build(const FarmTerrain& terrain, const CropManager& crops, const AnimalManager& animals, const DecorationManager& decorations,
    const ChestManager& chests, const GroundItems& groundItems) {
    FarmData result;
    for (const auto& [id, crop]: crops.getCrops())
        result.crops.push_back(crop.getPublicData());
    for (const auto& [id, animal]: animals.getAnimals())
        result.animals.push_back(animal->getPublicData());
    for (const auto& [id, decoration]: decorations.getDecorations())
        result.furniture.push_back(decoration.getPublicData());
    result.tilledSoil = crops.tilledPositions(terrain.getMap());
    result.loot = groundItems.getPublicData();
    const auto furniture = chests.furnitureData();
    result.furniture.insert(result.furniture.end(), furniture.begin(), furniture.end());
    return result;
}
