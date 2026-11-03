#include "farm_manager.h"

Farm::Farm(const FarmConfig& config):config(config), cropManager(config), animalManager(config) {}

bool Farm::setMap(const ZoneMap& map) {return !cropManager.hasTilledSoil() && space.setMap(map);}

const ZoneMap& Farm::getMap() const { return space.getMap(); }

bool Farm::canWalk(const Position& pos) const { return space.canWalk(pos); }

bool Farm::tillSoil(const Position& pos) {
    return cropManager.tillSoil(pos, space);
}

bool Farm::isTilled(const Position& pos) const {
    return cropManager.isTilled(pos, space);
}

bool Farm::plant(EntityId id, const Position& pos, PlantType type) {
    return cropManager.plant(id, pos, type, space);
}

bool Farm::waterCrop(EntityId id) { return cropManager.water(id); }

std::optional<FarmProduct> Farm::harvestCrop(EntityId id, double luck) {
    return cropManager.harvest(id, space, luck);
}

bool Farm::addAnimal(EntityId id, const Position& pos, AnimalType type) {
    return animalManager.add(id, pos, type, space);
}

bool Farm::moveAnimal(EntityId id, const Position& pos) {
    return animalManager.move(id, pos, space);
}

bool Farm::removeAnimal(EntityId id) { return animalManager.remove(id, space); }

std::optional<FarmProduct> Farm::collectFromAnimal(EntityId id) {
    return animalManager.collect(id);
}

bool Farm::placeDecoration(EntityId id, const Position& pos, ItemId item) {
    return decorationManager.place(id, pos, item, space);
}

std::optional<ItemId> Farm::removeDecoration(EntityId id) {
    return decorationManager.remove(id, space);
}

void Farm::update() {
    cropManager.update();
    animalManager.update();
}

const Crop* Farm::findCrop(EntityId id) const { return cropManager.find(id); }
const Animal* Farm::findAnimal(EntityId id) const { return animalManager.find(id); }
const Decoration* Farm::findDecoration(EntityId id) const { return decorationManager.find(id); }

const std::map<EntityId, Crop>& Farm::getCrops() const {
    return cropManager.getCrops();
}

const std::map<EntityId, Animal>& Farm::getAnimals() const {
    return animalManager.getAnimals();
}

const std::map<EntityId, Decoration>& Farm::getDecorations() const {
    return decorationManager.getDecorations();
}

const FarmConfig& Farm::getConfig() const { return config; }