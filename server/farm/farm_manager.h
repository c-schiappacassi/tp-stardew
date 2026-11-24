#pragma once

#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
#include <random>
#include <utility>
#include <vector>

#include "animals/animal_manager.h"
#include "chests/chest_manager.h"
#include "crops/crop_manager.h"
#include "decorations/decoration_manager.h"
#include "items/ground_items.h"
#include "persistence/farm_save_data.h"
#include "spatial/farm_terrain.h"

#include "farm_config.h"

class PlayerManager;
class PlayerSnapshot;
class EntityIdGenerator;

class FarmManager {
private:
    FarmConfig config;
    uint32_t seed;
    FarmTerrain terrain;
    CropManager crops;
    AnimalManager animals;
    DecorationManager decorations;
    ChestManager chests;
    GroundItems groundItems;

public:
    explicit FarmManager(const FarmConfig& config = FarmConfig{}, uint32_t seed = std::random_device{}());
    FarmManager(const FarmManager&) = delete;
    FarmManager& operator=(const FarmManager&) = delete;
    FarmManager(FarmManager&&) noexcept;
    FarmManager& operator=(FarmManager&&) noexcept;

    bool initialize(const FarmLayout& layout, EntityIdGenerator& entityIds);
    bool isInitialized() const;
    std::optional<WorldLocation> spawnFor(const PlayerSnapshot& player) const;
    bool setMap(const ZoneMap& map);
    bool setReservedPositions(const std::vector<Position>& positions);
    void setObstacles(const std::vector<CollisionBounds>& bounds);
    bool canWalk(const CollisionBounds& bounds) const;
    bool sameBaseCell(const Position& first, const Position& second) const;
    std::vector<TileData> backgroundData() const;

    bool useSelectedItem(PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& entityIds, std::chrono::steady_clock::time_point now);
    bool collectNearbyItems(PlayerManager& players, uint16_t playerId);
    bool tillSoil(const Position& position);
    bool isTilled(const Position& position) const;
    bool plant(EntityId id, const Position& position, CropType type);
    bool waterCrop(EntityId id);
    std::optional<FarmProduct> harvestCrop(EntityId id, double luck = 1.0);
    bool addAnimal(EntityId id, const Position& position, AnimalType type);
    bool moveAnimal(EntityId id, const Position& position);
    bool removeAnimal(EntityId id);
    std::optional<FarmProduct> collectFromAnimal(EntityId id);
    bool dropAnimalProduct(EntityId id, EntityIdGenerator& entityIds);
    bool buyAnimals(AnimalSpecies species, uint16_t quantity, std::optional<AnimalType> variant, EntityIdGenerator& ids, std::mt19937& variants);
    bool sellAnimals(AnimalSpecies species, uint16_t quantity);
    bool placeDecoration(EntityId id, const Position& position, ItemId item, Direction direction = Direction::Down);
    std::optional<ItemId> removeDecoration(EntityId id);
    bool placeChest(EntityId id, const Position& position, const std::map<uint8_t, Stack>& items = {});
    bool removeEmptyChest(EntityId id);
    std::optional<bool> interactWithChest(PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now);
    std::optional<OpenChestData> openChestFor(const PlayerSnapshot& player) const;
    bool depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command);
    bool withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command);
    void update(std::chrono::milliseconds elapsed, EntityIdGenerator& entityIds);
    void finishDay(std::chrono::milliseconds remainingTime, EntityIdGenerator& entityIds);
    std::size_t careCrops();
    std::size_t careAnimals(EntityIdGenerator& entityIds);

    std::optional<CropData> cropData(EntityId id) const;
    std::optional<AnimalData> animalData(EntityId id) const;
    std::optional<FurnitureData> decorationData(EntityId id) const;
    std::optional<ChestSnapshot> chestSnapshot(EntityId id) const;
    std::vector<Position> getTilledPositions() const;
    std::vector<std::pair<Position, uint64_t>> getEmptySoilElapsed() const;
    std::optional<uint32_t> animalBasePrice(AnimalSpecies species) const;
    std::optional<uint32_t> productBasePrice(ItemId item) const;
    FarmPrices prices() const;
    FarmConfig getConfig() const;
    FarmData getPublicData() const;
    FarmSaveData getPersistenceData() const;
    void restoreData(const FarmRestoreData& saved);
};
