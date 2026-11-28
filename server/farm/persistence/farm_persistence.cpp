#include "farm_persistence.h"

#include "server_constants.h"

#include <stdexcept>

#include "chests/chest_manager.h"
#include "farm/animals/animal_manager.h"
#include "farm/crops/crop_manager.h"
#include "farm/decorations/decoration_items.h"
#include "farm/decorations/decoration_manager.h"
#include "farm/farm_config.h"
#include "farm/queries/farm_status.h"
#include "farm/spatial/farm_terrain.h"
#include "items/ground_items.h"

FarmSaveData FarmPersistence::save(const FarmTerrain& terrain, const CropManager& crops, const AnimalManager& animals, const DecorationManager& decorations,
    const ChestManager& chests, const GroundItems& groundItems) {
    FarmSaveData saved;
    saved.farm = FarmStatus::build(terrain, crops, animals, decorations, chests, groundItems);
    saved.emptySoilElapsed = crops.emptySoilElapsedData(terrain.getMap());
    saved.chestInventories = chests.inventories();
    for (const auto& [id, crop]: crops.getCrops()) {
        const auto growth = crop.getGrowthElapsed().count();
        const auto water = crop.getRemainingWaterTime().count();
        if (growth < 0 || water < 0)
            throw std::runtime_error(server_constants::NEGATIVE_CROP_TIME_IN_PERSISTENCE_SNAPSHOT);
        saved.cropGrowthProgress[id] = static_cast<uint64_t>(growth);
        saved.cropWaterRemainingTime[id] = static_cast<uint64_t>(water);
        saved.harvestedCrops[id] = crop.isHarvested();
        saved.regrowingCrops[id] = crop.isRegrowing();
    }
    for (const auto& [id, animal]: animals.getAnimals()) {
        const auto elapsed = animal->getProductionElapsed().count();
        if (elapsed < 0)
            throw std::runtime_error(server_constants::NEGATIVE_ANIMAL_TIME_IN_PERSISTENCE_SNAPSHOT);
        saved.animalProductionProgress[id] = static_cast<uint64_t>(elapsed);
        saved.animalAvailableProducts[id] = animal->getAvailableProducts();
    }
    return saved;
}

void FarmPersistence::restore(const FarmRestoreData& saved, const FarmConfig& config, uint32_t seed, FarmTerrain& terrain, CropManager& crops, AnimalManager& animals,
    DecorationManager& decorations, ChestManager& chests, GroundItems& groundItems) {
    if (!terrain.isInitialized())
        throw std::runtime_error(server_constants::FARM_NOT_INITIALIZED);
    crops = CropManager(config);
    animals = AnimalManager(config, seed);
    decorations = DecorationManager();
    chests = ChestManager();
    groundItems = GroundItems();
    auto& space = terrain.getSpace();
    space.clearOccupants();
    for (const Position& position: saved.farm.tilledSoil) {
        if (!crops.tillSoil(position, terrain))
            throw std::runtime_error(server_constants::FAILED_TO_RESTORE_TILLED_SOIL);
    }
    for (const CropData& data: saved.farm.crops) {
        if (!crops.plant(data.id, data.pos, data.type, space))
            throw std::runtime_error(server_constants::FAILED_TO_RESTORE_CROP);
        const auto growth = saved.cropGrowthProgress.find(data.id);
        const auto water = saved.cropWaterRemainingTime.find(data.id);
        const auto harvested = saved.harvestedCrops.find(data.id);
        const auto regrowing = saved.regrowingCrops.find(data.id);
        const bool missingProgress = growth == saved.cropGrowthProgress.end() || water == saved.cropWaterRemainingTime.end() || harvested == saved.harvestedCrops.end();
        if (missingProgress || growth->second > static_cast<uint64_t>(INT64_MAX) || water->second > static_cast<uint64_t>(INT64_MAX))
            throw std::runtime_error(server_constants::MISSING_CROP_PROGRESS);
        const bool wasRegrowing = regrowing != saved.regrowingCrops.end() && regrowing->second;
        crops.findMutable(data.id)->restoreProgress(std::chrono::milliseconds{static_cast<int64_t>(growth->second)},
            std::chrono::milliseconds{static_cast<int64_t>(water->second)}, harvested->second, wasRegrowing);
    }
    for (const auto& [position, elapsed]: saved.emptySoilElapsed) {
        if (elapsed > static_cast<uint64_t>(INT64_MAX) || !crops.restoreEmptySoilElapsed(position, std::chrono::milliseconds{static_cast<int64_t>(elapsed)}, space)) {
            throw std::runtime_error(server_constants::INVALID_SAVED_EMPTY_SOIL_TIME);
        }
    }
    for (const AnimalData& data: saved.farm.animals) {
        if (!animals.add(data.id, data.pos, data.type, space))
            throw std::runtime_error(server_constants::FAILED_TO_RESTORE_ANIMAL);
        const auto production = saved.animalProductionProgress.find(data.id);
        const auto products = saved.animalAvailableProducts.find(data.id);
        if (production == saved.animalProductionProgress.end() || products == saved.animalAvailableProducts.end() || production->second > static_cast<uint64_t>(INT64_MAX))
            throw std::runtime_error(server_constants::MISSING_ANIMAL_PROGRESS);
        animals.findMutable(data.id)->restoreProgress(std::chrono::milliseconds{static_cast<int64_t>(production->second)}, products->second);
    }
    for (const FurnitureData& data: saved.farm.furniture) {
        if (data.type != FurnitureId::Chest)
            continue;
        const auto contents = saved.chestInventories.find(data.id);
        if (contents == saved.chestInventories.end() || !chests.place(data.id, data.pos, space, contents->second))
            throw std::runtime_error(server_constants::FAILED_TO_RESTORE_CHEST);
    }
    auto loot = saved.farm.loot;
    for (const FurnitureData& data: saved.farm.furniture) {
        if (data.type == FurnitureId::Chest)
            continue;
        const ItemId item = decoration_items::itemFor(data.type);
        if (!Decoration::isDecoration(item) || data.dir < Direction::Up || data.dir > Direction::Right)
            throw std::runtime_error(server_constants::FAILED_TO_RESTORE_DECORATION);
        if (!decorations.place(data.id, data.pos, item, space, data.dir))
            loot.push_back(LootData{data.id, data.pos, item, 1, Quality::None, LootKind::Item});
    }
    groundItems.restoreItems(loot);
}
