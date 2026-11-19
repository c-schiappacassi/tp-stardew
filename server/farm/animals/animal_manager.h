#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <vector>
#include <random>
#include "animal.h"
#include "animal_definition.h"
#include "farm/ports/farm_placement.h"

class FarmConfig;
class GroundItems;
class EntityIdGenerator;

class AnimalManager {
private:
    std::vector<AnimalDefinition> definitions;
    std::map<EntityId, std::unique_ptr<Animal>> animals;
    std::mt19937 random;
    const AnimalDefinition* findDefinition(AnimalSpecies species) const;
    std::size_t countSpecies(AnimalSpecies species) const;

public:
    explicit AnimalManager(const FarmConfig& config, uint32_t seed = std::random_device{}());

    void update(std::chrono::milliseconds elapsed, FarmPlacement& placement,GroundItems& groundItems, EntityIdGenerator& entityIds);
    bool add(EntityId id, const Position& position, AnimalType type, FarmPlacement& placement);
    bool move(EntityId id, const Position& position, FarmPlacement& placement);
    bool remove(EntityId id, FarmPlacement& placement);
    std::optional<FarmProduct> collect(EntityId id);
    Animal* findMutable(EntityId id);
    const Animal* find(EntityId id) const;
    std::vector<const Animal*> getAnimals() const;
};
