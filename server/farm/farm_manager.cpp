#include "farm_manager.h"

#include <stdexcept>
#include <utility>

#include "entities/entity_id_generator.h"
#include "player/player.h"
#include "farm_status.h"
#include "loading/farm_loader.h"

FarmManager::FarmManager(const FarmConfig& config, uint32_t seed):config(config), seed(seed), cropManager(config), animalManager(config, seed) {}

bool FarmManager::initialize(const FarmLayout& layout, EntityIdGenerator& entityIds) {
    FarmManager prepared{config, seed};
    prepared.background = layout.background;
    for (const auto& tile : layout.background) {
        if (tile.textureId == static_cast<uint16_t>(TileTextureId::Dirt)) {
            prepared.dirtCells.emplace(tile.tile.getY(), tile.tile.getX());
        }
    }
    auto preparedIds = entityIds;
    if (!FarmLoader{}.load(layout, prepared, preparedIds)) {
        return false;
    }
    prepared.spawnPosition = layout.spawn;
    prepared.initialized = true;
    *this = std::move(prepared);
    entityIds = preparedIds;
    return true;
}

bool FarmManager::isInitialized() const { return initialized; }

bool FarmManager::spawnPlayer(Player& player) const {
    if (!initialized) {
        return false;
    }
    const auto collision = player.getCollisionBounds(spawnPosition);
    if (space.canWalk(collision)) {
        return player.restoreLocation(Zone::Farm, 0, spawnPosition);
    }
    for (const auto& position : space.getMap().walkableBasePositions()) {
        if (space.canWalk(player.getCollisionBounds(position))) {
            return player.restoreLocation(Zone::Farm, 0, position);
        }
    }
    return false;
}

bool FarmManager::useSelectedItem(Player& player, Direction direction, EntityIdGenerator& entityIds,
        std::chrono::steady_clock::time_point now) {
    return interactions.interact(player, *this, entityIds, direction, now);
}

bool FarmManager::collectNearbyItems(Player& player) {
    if (!initialized || player.getZone() != Zone::Farm) {
        return false;
    }
    return groundItems.collect(player, [&](const Position& position) {
        return FarmTargeting{}.hasClearPath(player.getPosition(), position, getMap());
    });
}

bool FarmManager::setReservedPositions(const std::vector<Position>& positions) {
    if (cropManager.hasTilledSoil()) {
        return false;
    }
    return space.setReservedPositions(positions);
}

void FarmManager::setObstacles(const std::vector<CollisionBounds>& bounds) {
    space.setObstacles(bounds);
}

FarmData FarmManager::getPublicData() const { return FarmStatus::build(*this); }

std::vector<Position> FarmManager::getTilledPositions() const {
    std::vector<Position> result;
    for (const auto& cell : cropManager.getTilledSoil()) {
        const auto base = space.getMap().baseAt(cell);
        if (base) {
            result.push_back(*base);
        }
    }
    return result;
}

bool FarmManager::setMap(const ZoneMap& map) {
    if (cropManager.hasTilledSoil()) {
        return false;
    }
    return space.setMap(map);
}

const ZoneMap& FarmManager::getMap() const { return space.getMap(); }

const std::vector<FarmBackgroundTile>& FarmManager::getBackground() const {
    return background;
}

bool FarmManager::canWalk(const CollisionBounds& bounds) const { return space.canWalk(bounds); }

bool FarmManager::tillSoil(const Position& position) {
    const auto cell = space.cellAt(position);
    if (!cell || !dirtCells.contains(*cell)) return false;
    return cropManager.tillSoil(position, space);
}

bool FarmManager::isTilled(const Position& position) const {
    return cropManager.isTilled(position, space);
}

bool FarmManager::plant(EntityId id, const Position& position, CropType type) {
    return cropManager.plant(id, position, type, space);
}

bool FarmManager::waterCrop(EntityId id) { return cropManager.water(id); }

std::optional<FarmProduct> FarmManager::harvestCrop(EntityId id, double luck) {
    return cropManager.harvest(id, space, luck);
}

bool FarmManager::addAnimal(EntityId id, const Position& position, AnimalType type) {
    return animalManager.add(id, position, type, space);
}

bool FarmManager::moveAnimal(EntityId id, const Position& position) {
    return animalManager.move(id, position, space);
}

bool FarmManager::removeAnimal(EntityId id) { return animalManager.remove(id, space); }

std::optional<FarmProduct> FarmManager::collectFromAnimal(EntityId id) {
    return animalManager.collect(id);
}

bool FarmManager::placeDecoration(EntityId id, const Position& position, ItemId item, Direction direction) {
    return decorationManager.place(id, position, item, space, direction);
}

std::optional<ItemId> FarmManager::removeDecoration(EntityId id) {
    return decorationManager.remove(id, space);
}

void FarmManager::update(std::chrono::milliseconds elapsed, EntityIdGenerator& entityIds) {
    if (elapsed.count() < 0) {
        throw std::invalid_argument("Negative time");
    }
    if (elapsed.count() == 0) {
        return;
    }
    cropManager.update(elapsed);
    animalManager.update(elapsed, space, groundItems, entityIds);
}

void FarmManager::finishDay(std::chrono::milliseconds remainingTime, EntityIdGenerator& entityIds) {
    update(remainingTime, entityIds);
    cropManager.finishDay();
}

const Crop* FarmManager::findCrop(EntityId id) const { return cropManager.find(id); }
const Animal* FarmManager::findAnimal(EntityId id) const { return animalManager.find(id); }
const Decoration* FarmManager::findDecoration(EntityId id) const {
    return decorationManager.find(id);
}

const std::map<EntityId, Crop>& FarmManager::getCrops() const {
    return cropManager.getCrops();
}

std::vector<const Animal*> FarmManager::getAnimals() const {
    return animalManager.getAnimals();
}
const std::map<EntityId, Decoration>& FarmManager::getDecorations() const {
    return decorationManager.getDecorations();
}

const FarmConfig& FarmManager::getConfig() const { return config; }

GroundItems& FarmManager::getGroundItems() { return groundItems; }
const GroundItems& FarmManager::getGroundItems() const { return groundItems; }

void FarmManager::restoreData(const FarmData& saved,
    const std::map<uint32_t, uint64_t>& growth,
    const std::map<uint32_t, uint64_t>& water,
    const std::map<uint32_t, bool>& harvested,
    const std::map<uint32_t, uint64_t>& production,
    const std::map<uint32_t, uint32_t>& products) {
    if (!initialized) throw std::runtime_error("Farm not initialized");
    cropManager = CropManager(config);
    animalManager = AnimalManager(config, seed);
    decorationManager = DecorationManager();
    groundItems = GroundItems();
    space.clearOccupants();
    for (const Position& position : saved.tilledSoil) {
        if (!tillSoil(position)) throw std::runtime_error("Failed to restore tilled soil");
    }
    for (const CropData& data : saved.crops) {
        if (!plant(data.id, data.pos, data.type)) throw std::runtime_error("Failed to restore crop");
        const auto g = growth.find(data.id);
        const auto w = water.find(data.id);
        const auto h = harvested.find(data.id);
        if (g == growth.end() || w == water.end() || h == harvested.end() ||
            g->second > static_cast<uint64_t>(INT64_MAX) ||
            w->second > static_cast<uint64_t>(INT64_MAX)) throw std::runtime_error("Missing crop progress");
        cropManager.findMutable(data.id)->restoreProgress(
            std::chrono::milliseconds{static_cast<int64_t>(g->second)},
            std::chrono::milliseconds{static_cast<int64_t>(w->second)}, h->second);
    }
    for (const AnimalData& data : saved.animals) {
        if (!addAnimal(data.id, data.pos, data.type)) throw std::runtime_error("Failed to restore animal");
        const auto t = production.find(data.id);
        const auto n = products.find(data.id);
        if (t == production.end() || n == products.end() ||
            t->second > static_cast<uint64_t>(INT64_MAX)) throw std::runtime_error("Missing animal progress");
        animalManager.findMutable(data.id)->restoreProgress(
            std::chrono::milliseconds{static_cast<int64_t>(t->second)}, n->second);
    }
    for (const FurnitureData& data : saved.furniture) {
        const ItemId item = static_cast<ItemId>(
            static_cast<uint16_t>(ItemId::Chair) + static_cast<uint16_t>(data.type));
        if (!placeDecoration(data.id, data.pos, item, data.dir))
            throw std::runtime_error("Failed to restore decoration");
    }
    groundItems.restoreItems(saved.loot);
}
