#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <utility>
#include <vector>

#include "chests/loading/chest_spawn.h"
#include "dtos.h"
#include "entities/collision_box.h"
#include "entities/entity_id.h"
#include "farm/animals/animal_movement_config.h"
#include "farm/animals/animal_species.h"
#include "position.h"
#include "protocol_types.h"
#include "world/world_types.h"
#include "world/zone_map.h"

// Products
struct FarmProduct {
    ItemId item = ItemId::Strawberry;
    uint32_t quantity = 1;
    Quality quality = Quality::None;
};

// Targeting
enum class FarmProductSource { Crop, Animal };

struct FarmCollectionTarget {
    EntityId id;
    FarmProductSource source;
    Position position;
    FarmProduct product;
};

// Crops & Animals definitions
struct CropDefinition {
    CropType type;
    std::chrono::milliseconds growthTime;
    std::chrono::milliseconds wateringInterval;
    std::chrono::milliseconds regrowthTime;
    uint32_t toleratedMissedWaterings;
    uint32_t basePrice;
};

struct AnimalDefinition {
    AnimalSpecies species;
    std::chrono::milliseconds productionInterval;
    std::size_t maxCount;
    uint32_t basePrice;
    uint32_t productPrice;
    AnimalMovementConfig movement{};
    CollisionBox collision;
};

// Prices
struct FarmProductPrice {
    ItemId item;
    uint32_t basePrice;
};

struct FarmAnimalPrice {
    AnimalSpecies species;
    uint32_t basePrice;
};

struct FarmPrices {
    std::vector<FarmProductPrice> products;
    std::vector<FarmAnimalPrice> animals;
};

// Layout
struct FarmBackgroundTile {
    Position tile{0, 0};
    uint16_t textureId = 0;
};

struct FarmAnimalSpawn {
    Position base;
    AnimalType type;
};

struct FarmCropSpawn {
    Position base;
    CropType type;
    bool watered = false;
};

struct FarmDecorationSpawn {
    Position base;
    ItemId item;
};

using FarmChestSpawn = ChestSpawn;

struct FarmLayout {
    ZoneMap terrain;
    Position spawn{0, 0};
    std::vector<Position> reservedPositions;
    std::vector<Position> tilledSoil;
    std::vector<FarmAnimalSpawn> animals;
    std::vector<FarmCropSpawn> crops;
    std::vector<FarmDecorationSpawn> decorations;
    std::vector<FarmChestSpawn> chests;
    std::vector<FarmBackgroundTile> background;
    std::optional<Position> mineDoor;
    std::optional<Position> mineReturn;
    std::optional<Position> townDoor;
    std::vector<Position> extraTownDoors;
    std::optional<Position> townReturn;
    std::vector<InteriorLayout> interiors;
    uint8_t playerHouses = 4;
};

// Persistence
struct FarmRestoreData {
    const FarmData& farm;
    const std::map<uint32_t, uint64_t>& cropGrowthProgress;
    const std::map<uint32_t, uint64_t>& cropWaterRemainingTime;
    const std::map<uint32_t, bool>& harvestedCrops;
    const std::map<uint32_t, bool>& regrowingCrops;
    const std::map<uint32_t, uint64_t>& animalProductionProgress;
    const std::map<uint32_t, uint32_t>& animalAvailableProducts;
    const std::vector<std::pair<Position, uint64_t>>& emptySoilElapsed;
    const std::map<uint32_t, std::map<uint8_t, Stack>>& chestInventories;
};

struct FarmSaveData {
    FarmData farm;
    std::map<uint32_t, uint64_t> cropGrowthProgress;
    std::map<uint32_t, uint64_t> cropWaterRemainingTime;
    std::map<uint32_t, bool> harvestedCrops;
    std::map<uint32_t, bool> regrowingCrops;
    std::map<uint32_t, uint64_t> animalProductionProgress;
    std::map<uint32_t, uint32_t> animalAvailableProducts;
    std::vector<std::pair<Position, uint64_t>> emptySoilElapsed;
    std::map<uint32_t, std::map<uint8_t, Stack>> chestInventories;

    FarmRestoreData view() const;
};
