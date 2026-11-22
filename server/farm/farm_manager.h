#pragma once

#include <map>
#include <optional>
#include <random>
#include <set>
#include <utility>
#include <vector>
#include "animals/animal_manager.h"
#include "crops/crop_manager.h"
#include "decorations/decoration_manager.h"
#include "farm_config.h"
#include "interactions/farm_interactions.h"
#include "loading/farm_layout.h"
#include "spatial/farm_space.h"
#include "items/ground_items.h"

class Player;
class EntityIdGenerator;

class FarmManager {
private:
    FarmConfig config;
    uint32_t seed;
    bool initialized = false;
    Position spawnPosition{0, 0};
    std::vector<FarmBackgroundTile> background;
    std::set<ZoneMap::Cell> dirtCells;
    FarmSpace space;
    CropManager cropManager;
    AnimalManager animalManager;
    DecorationManager decorationManager;
    FarmInteractions interactions;
    GroundItems groundItems;

public:
    explicit FarmManager(const FarmConfig& config = FarmConfig{},
        uint32_t seed = std::random_device{}());

    bool initialize(const FarmLayout& layout, EntityIdGenerator& entityIds);
    bool isInitialized() const;
    bool spawnPlayer(Player& player) const;
    bool useSelectedItem(Player& player, Direction direction, EntityIdGenerator& entityIds,std::chrono::steady_clock::time_point now);
    bool collectNearbyItems(Player& player);
    bool setMap(const ZoneMap& map);
    bool setReservedPositions(const std::vector<Position>& positions);
    void setObstacles(const std::vector<CollisionBounds>& bounds);
    const ZoneMap& getMap() const;
    const std::vector<FarmBackgroundTile>& getBackground() const;
    bool canWalk(const CollisionBounds& bounds) const;
    bool tillSoil(const Position& position);
    bool isTilled(const Position& position) const;
    bool plant(EntityId id, const Position& position, CropType type);
    bool addAnimal(EntityId id, const Position& position, AnimalType type);
    bool moveAnimal(EntityId id, const Position& position);
    bool removeAnimal(EntityId id);
    bool placeDecoration(EntityId id, const Position& position, ItemId item, Direction direction = Direction::Down);
    std::optional<ItemId> removeDecoration(EntityId id);
    bool waterCrop(EntityId id);
    std::optional<FarmProduct> harvestCrop(EntityId id, double luck = 1.0);
    std::optional<FarmProduct> collectFromAnimal(EntityId id);
    bool dropAnimalProduct(EntityId id, EntityIdGenerator& entityIds);
    void update(std::chrono::milliseconds elapsed, EntityIdGenerator& entityIds);
    void finishDay(std::chrono::milliseconds remainingTime, EntityIdGenerator& entityIds);
    std::size_t careCrops();
    std::size_t careAnimals(EntityIdGenerator& entityIds);
    GroundItems& getGroundItems();
    const GroundItems& getGroundItems() const;
    FarmData getPublicData() const;
    void restoreData(const FarmData& saved,
                     const std::map<uint32_t, uint64_t>& growth,
                     const std::map<uint32_t, uint64_t>& water,
                     const std::map<uint32_t, bool>& harvested,
                     const std::map<uint32_t, uint64_t>& production,
                     const std::map<uint32_t, uint32_t>& products,
                     const std::map<uint32_t, bool>& regrowing = {},
                     const std::vector<std::pair<Position, uint64_t>>& emptySoilElapsed = {});
    std::vector<Position> getTilledPositions() const;
    std::vector<std::pair<Position, uint64_t>> getEmptySoilElapsed() const;
    const Crop* findCrop(EntityId id) const;
    const Animal* findAnimal(EntityId id) const;
    const Decoration* findDecoration(EntityId id) const;
    const std::map<EntityId, Crop>& getCrops() const;
    std::vector<const Animal*> getAnimals() const;
    const std::map<EntityId, Decoration>& getDecorations() const;
    const FarmConfig& getConfig() const;
};
