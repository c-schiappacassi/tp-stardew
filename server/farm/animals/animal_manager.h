#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <vector>
#include <random>
#include "animal.h"
#include "animal_definition.h"
#include "entities/placement/placement.h"

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

    void update(std::chrono::milliseconds elapsed, Placement& placement,GroundItems& groundItems, EntityIdGenerator& entityIds);
    bool add(EntityId id, const Position& position, AnimalType type, Placement& placement);
    bool move(EntityId id, const Position& position, Placement& placement);
    bool remove(EntityId id, Placement& placement);
    std::optional<FarmProduct> collect(EntityId id);
    bool dropProduction(EntityId id, GroundItems& groundItems, EntityIdGenerator& entityIds);
    Animal* findMutable(EntityId id);
    const Animal* find(EntityId id) const;
    std::optional<AnimalData> dataFor(EntityId id) const;
    const std::map<EntityId, std::unique_ptr<Animal>>& getAnimals() const;
    std::size_t care(Placement& placement, GroundItems& groundItems, EntityIdGenerator& entityIds);
    bool buy(AnimalSpecies species, uint16_t quantity, std::optional<AnimalType> variant, Placement& placement, const ZoneMap& map, EntityIdGenerator& ids, std::mt19937& variants);
    bool sell(AnimalSpecies species, uint16_t quantity, Placement& placement);
};
