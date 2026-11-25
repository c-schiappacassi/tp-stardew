#include "farm_manager.h"

#include "server_constants.h"

#include <stdexcept>
#include <utility>

#include "entities/entity_id_generator.h"
#include "interactions/farm_interactions.h"
#include "loading/farm_loader.h"
#include "persistence/farm_persistence.h"
#include "queries/farm_status.h"

FarmManager::FarmManager(const FarmConfig& config, uint32_t seed): config(config), seed(seed), crops(config), animals(config, seed) {}

FarmManager::FarmManager(FarmManager&&) noexcept = default;
FarmManager& FarmManager::operator=(FarmManager&&) noexcept = default;

bool FarmManager::initialize(const FarmLayout& layout, EntityIdGenerator& entityIds) {
    FarmManager prepared{config, seed};
    auto preparedIds = entityIds;
    if (!FarmLoader{}.load(layout, prepared.terrain, prepared.crops, prepared.animals, prepared.decorations, prepared.chests, preparedIds))
        return false;
    *this = std::move(prepared);
    entityIds = preparedIds;
    return true;
}

bool FarmManager::isInitialized() const { return terrain.isInitialized(); }
std::optional<WorldLocation> FarmManager::spawnFor(const PlayerSnapshot& player) const { return terrain.spawnFor(player); }
bool FarmManager::setMap(const ZoneMap& map) { return !crops.hasTilledSoil() && terrain.setMap(map); }
bool FarmManager::setReservedPositions(const std::vector<Position>& positions) { return !crops.hasTilledSoil() && terrain.setReservedPositions(positions); }
void FarmManager::setObstacles(const std::vector<CollisionBounds>& bounds) { terrain.setObstacles(bounds); }
bool FarmManager::canWalk(const CollisionBounds& bounds) const { return terrain.canWalk(bounds); }
bool FarmManager::sameBaseCell(const Position& first, const Position& second) const { return terrain.sameBaseCell(first, second); }
std::vector<TileData> FarmManager::backgroundData() const { return terrain.backgroundData(); }

bool FarmManager::useSelectedItem(PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& entityIds, std::chrono::steady_clock::time_point now) {
    FarmInteractions interactions{terrain, crops, animals, decorations, chests, groundItems};
    return interactions.interact(players, playerId, entityIds, direction, now);
}

bool FarmManager::collectNearbyItems(PlayerManager& players, uint16_t playerId) {
    return FarmInteractions{terrain, crops, animals, decorations, chests, groundItems}.collectNearbyItems(players, playerId);
}

bool FarmManager::tillSoil(const Position& position) { return crops.tillSoil(position, terrain); }
bool FarmManager::isTilled(const Position& position) const { return crops.isTilled(position, terrain.getSpace()); }
bool FarmManager::plant(EntityId id, const Position& position, CropType type) { return crops.plant(id, position, type, terrain.getSpace()); }
bool FarmManager::waterCrop(EntityId id) { return crops.water(id); }
std::optional<FarmProduct> FarmManager::harvestCrop(EntityId id, double luck) { return crops.harvest(id, terrain.getSpace(), luck); }
bool FarmManager::addAnimal(EntityId id, const Position& position, AnimalType type) { return animals.add(id, position, type, terrain.getSpace()); }
bool FarmManager::moveAnimal(EntityId id, const Position& position) { return animals.move(id, position, terrain.getSpace()); }
bool FarmManager::removeAnimal(EntityId id) { return animals.remove(id, terrain.getSpace()); }
std::optional<FarmProduct> FarmManager::collectFromAnimal(EntityId id) { return animals.collect(id); }
bool FarmManager::dropAnimalProduct(EntityId id, EntityIdGenerator& entityIds) { return animals.dropProduction(id, groundItems, entityIds); }

bool FarmManager::buyAnimals(AnimalSpecies species, uint16_t quantity, std::optional<AnimalType> variant, EntityIdGenerator& ids, std::mt19937& variants) {
    return isInitialized() && animals.buy(species, quantity, variant, terrain.getSpace(), terrain.getMap(), ids, variants);
}

bool FarmManager::sellAnimals(AnimalSpecies species, uint16_t quantity) { return isInitialized() && animals.sell(species, quantity, terrain.getSpace()); }

bool FarmManager::placeDecoration(EntityId id, const Position& position, ItemId item, Direction direction) {
    return decorations.place(id, position, item, terrain.getSpace(), direction);
}

std::optional<ItemId> FarmManager::removeDecoration(EntityId id) { return decorations.remove(id, terrain.getSpace()); }
bool FarmManager::placeChest(EntityId id, const Position& position, const std::map<uint8_t, Stack>& items) { return chests.place(id, position, terrain.getSpace(), items); }
bool FarmManager::removeEmptyChest(EntityId id) { return chests.removeEmpty(id, terrain.getSpace()); }

std::optional<bool> FarmManager::interactWithChest(
    PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) {
    return chests.interact(players, playerId, terrain.getSpace(), terrain.getMap(), groundItems, direction, ids, now);
}

std::optional<OpenChestData> FarmManager::openChestFor(const PlayerSnapshot& player) const { return chests.contentsFor(player, terrain.getMap()); }

bool FarmManager::depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command) {
    return chests.deposit(players, playerId, terrain.getMap(), command);
}

bool FarmManager::withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command) {
    return chests.withdraw(players, playerId, terrain.getMap(), command);
}

void FarmManager::update(std::chrono::milliseconds elapsed, EntityIdGenerator& entityIds) {
    if (elapsed.count() < 0)
        throw std::invalid_argument(server_constants::NEGATIVE_TIME);
    if (elapsed.count() == 0)
        return;
    crops.update(elapsed);
    animals.update(elapsed, terrain.getSpace(), groundItems, entityIds);
}

void FarmManager::finishDay(std::chrono::milliseconds remainingTime, EntityIdGenerator& entityIds) {
    update(remainingTime, entityIds);
    crops.finishDay();
}

std::size_t FarmManager::careCrops() { return crops.care(); }
std::size_t FarmManager::careAnimals(EntityIdGenerator& entityIds) { return animals.care(terrain.getSpace(), groundItems, entityIds); }
std::optional<CropData> FarmManager::cropData(EntityId id) const { return crops.dataFor(id); }
std::optional<AnimalData> FarmManager::animalData(EntityId id) const { return animals.dataFor(id); }
std::optional<FurnitureData> FarmManager::decorationData(EntityId id) const { return decorations.dataFor(id); }
std::optional<ChestSnapshot> FarmManager::chestSnapshot(EntityId id) const { return chests.snapshot(id); }
std::vector<Position> FarmManager::getTilledPositions() const { return crops.tilledPositions(terrain.getMap()); }
std::vector<std::pair<Position, uint64_t>> FarmManager::getEmptySoilElapsed() const { return crops.emptySoilElapsedData(terrain.getMap()); }
std::optional<uint32_t> FarmManager::animalBasePrice(AnimalSpecies species) const { return config.animalBasePrice(species); }
std::optional<uint32_t> FarmManager::productBasePrice(ItemId item) const { return config.productBasePrice(item); }
FarmPrices FarmManager::prices() const { return config.prices(); }
FarmConfig FarmManager::getConfig() const { return config; }
FarmData FarmManager::getPublicData() const { return FarmStatus::build(terrain, crops, animals, decorations, chests, groundItems); }
FarmSaveData FarmManager::getPersistenceData() const { return FarmPersistence::save(terrain, crops, animals, decorations, chests, groundItems); }

void FarmManager::restoreData(const FarmRestoreData& saved) { FarmPersistence::restore(saved, config, seed, terrain, crops, animals, decorations, chests, groundItems); }
