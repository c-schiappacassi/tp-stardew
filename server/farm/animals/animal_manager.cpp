#include "animal_manager.h"
#include "farm/farm_config.h"

#include <algorithm>
#include <utility>

#include "animal_factory.h"
#include "farm/spatial/farm_reservation.h"
#include "items/ground_items.h"

namespace {
Quality rollProductQuality(std::mt19937& random) {
    return static_cast<Quality>(std::uniform_int_distribution<int>{
        static_cast<int>(Quality::Base), static_cast<int>(Quality::Perfect)}(random));
}
}

AnimalManager::AnimalManager(const FarmConfig& config, uint32_t seed):
    definitions(config.animals), random(seed) {
    config.validate();
}

const AnimalDefinition* AnimalManager::findDefinition(AnimalSpecies species) const {
    for (const auto& definition : definitions) {
        if (definition.species == species) {
            return &definition;
        }
    }

    return nullptr;
}

std::size_t AnimalManager::countSpecies(AnimalSpecies species) const {
    return static_cast<std::size_t>(std::count_if(
        animals.begin(), animals.end(),
        [species](const auto& entry) {
            return entry.second->getSpecies() == species;
        }));
}

bool AnimalManager::add(EntityId id, const Position& position, AnimalType type,
    FarmPlacement& placement) {
    const auto species = animalSpeciesForType(type);
    if (!species) {
        return false;
    }
    const auto* definition = findDefinition(*species);

    if (!definition || id == 0 || animals.contains(id) ||
        placement.contains(id) || !placement.isAvailable(definition->collision.at(position)) ||
        countSpecies(*species) >= definition->maxCount) {
        return false;
    }

    auto animal = AnimalFactory::create(id, position, type, *definition);

    if (!animal) {
        return false;
    }
    FarmReservation reservation{placement, id, position, definition->collision};
    if (!reservation.isReserved() || !animals.emplace(id, std::move(animal)).second) {
        return false;
    }
    reservation.commit();
    return true;
}

bool AnimalManager::move(EntityId id, const Position& position, FarmPlacement& placement) {
    const auto it = animals.find(id);
    if (it == animals.end()) {
        return false;
    }
    return it->second->moveTo(position, placement);
}

bool AnimalManager::remove(EntityId id, FarmPlacement& placement) {
    const auto it = animals.find(id);
    if (it == animals.end() || !placement.release(id)) {
        return false;
    }
    animals.erase(it);
    return true;
}

std::optional<FarmProduct> AnimalManager::collect(EntityId id) {
    const auto it = animals.find(id);

    if (it == animals.end()) {
        return std::nullopt;
    }
    auto product = it->second->collect();
    if (product) {
        product->quality = rollProductQuality(random);
    }
    return product;
}

bool AnimalManager::dropProduction(EntityId id, GroundItems& groundItems, EntityIdGenerator& entityIds) {
    const auto it = animals.find(id);
    if (it == animals.end()) {
        return false;
    }
    auto product = it->second->getProduction();
    if (!product) {
        return false;
    }
    auto preparedRandom = random;
    product->quality = rollProductQuality(preparedRandom);
    if (!groundItems.drop(it->second->getPosition(), product->item, product->quantity,
                          product->quality, entityIds)) {
        return false;
    }
    it->second->collect();
    random = preparedRandom;
    return true;
}

void AnimalManager::update(std::chrono::milliseconds elapsed, FarmPlacement& placement,GroundItems& groundItems, EntityIdGenerator& entityIds) {
    for (auto& [id, animal] : animals) {
        animal->updateProduction(elapsed);
        if (animal->dropsProductsAutomatically()) {
            dropProduction(id, groundItems, entityIds);
        }
        animal->advanceMovement(placement, random);
    }
}

const Animal* AnimalManager::find(EntityId id) const {
    const auto it = animals.find(id);

    if (it == animals.end()) {
        return nullptr;
    } else {
        return it->second.get();
    }
}

std::vector<const Animal*> AnimalManager::getAnimals() const {
    std::vector<const Animal*> result;
    result.reserve(animals.size());

    for (const auto& [id, animal] : animals) {
        result.push_back(animal.get());
    }

    return result;
}

Animal* AnimalManager::findMutable(EntityId id) {
    const auto it = animals.find(id);
    return it == animals.end() ? nullptr : it->second.get();
}
