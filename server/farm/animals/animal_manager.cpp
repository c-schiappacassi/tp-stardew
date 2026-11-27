#include "animal_manager.h"
#include "farm/farm_config.h"

#include <algorithm>
#include <utility>

#include "animal_factory.h"
#include "entities/entity_id_generator.h"
#include "entities/placement/placement_reservation.h"
#include "items/ground_items.h"

Quality AnimalManager::rollProductQuality(std::mt19937& random) {
    return static_cast<Quality>(std::uniform_int_distribution<int>{
        static_cast<int>(Quality::Base), static_cast<int>(Quality::Perfect)}(random));
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
    Placement& placement) {
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
    PlacementReservation reservation{placement, id, position, definition->collision};
    if (!reservation.isReserved() || !animals.emplace(id, std::move(animal)).second) {
        return false;
    }
    reservation.commit();
    return true;
}

bool AnimalManager::move(EntityId id, const Position& position, Placement& placement) {
    const auto it = animals.find(id);
    if (it == animals.end()) {
        return false;
    }
    return it->second->moveTo(position, placement);
}

bool AnimalManager::remove(EntityId id, Placement& placement) {
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

void AnimalManager::update(std::chrono::milliseconds elapsed, Placement& placement,GroundItems& groundItems, EntityIdGenerator& entityIds) {
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

const std::map<EntityId, std::unique_ptr<Animal>>& AnimalManager::getAnimals() const { return animals; }

Animal* AnimalManager::findMutable(EntityId id) {
    const auto it = animals.find(id);
    return it == animals.end() ? nullptr : it->second.get();
}

std::size_t AnimalManager::care(Placement& placement, GroundItems& groundItems, EntityIdGenerator& entityIds) {
    for (auto& [id, animal]: animals)
        animal->restoreProgress(std::chrono::milliseconds{0}, std::max<uint32_t>(1, animal->getAvailableProducts()));
    update(std::chrono::milliseconds{0}, placement, groundItems, entityIds);
    return animals.size();
}

bool AnimalManager::buy(AnimalSpecies species, uint16_t quantity, std::optional<AnimalType> variant,
    Placement& placement, const ZoneMap& map, EntityIdGenerator& ids, std::mt19937& variants) {
    if (quantity == 0)
        return false;
    const auto types = animalTypesForSpecies(species);
    if (types.empty() || (variant && animalSpeciesForType(*variant) != species))
        return false;
    const auto* definition = findDefinition(species);
    if (!definition)
        return false;
    const auto count = countSpecies(species);
    if (count > definition->maxCount || quantity > definition->maxCount - count)
        return false;
    std::vector<EntityId> added;
    added.reserve(quantity);
    auto preparedIds = ids;
    auto preparedRandom = variants;
    std::uniform_int_distribution<std::size_t> chooseType(0, types.size() - 1);
    const auto nextType = [&] { return variant ? *variant : types[chooseType(preparedRandom)]; };
    const auto rollback = [&] {
        for (const auto id: added) remove(id, placement);
    };
    AnimalType type = nextType();
    try {
        for (const auto& position: map.walkableBasePositions()) {
            const auto id = preparedIds.getNextId();
            if (!add(id, position, type, placement))
                continue;
            added.push_back(id);
            preparedIds.generate();
            if (added.size() == quantity) {
                ids = preparedIds;
                variants = preparedRandom;
                return true;
            }
            type = nextType();
        }
    } catch (...) {
        rollback();
        throw;
    }
    rollback();
    return false;
}

bool AnimalManager::sell(AnimalSpecies species, uint16_t quantity, Placement& placement) {
    if (quantity == 0)
        return false;
    std::vector<EntityId> selected;
    for (const auto& [id, animal]: animals) {
        if (animal->getSpecies() == species)
            selected.push_back(animal->getId());
        if (selected.size() == quantity)
            break;
    }
    if (selected.size() != quantity)
        return false;
    for (const auto id: selected) remove(id, placement);
    return true;
}

std::optional<AnimalData> AnimalManager::dataFor(EntityId id) const {
    const auto* entity = find(id);
    return entity ? std::optional<AnimalData>{entity->getPublicData()} : std::nullopt;
}
