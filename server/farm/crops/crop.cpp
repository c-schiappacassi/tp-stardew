#include "crop.h"

#include <stdexcept>

Crop::Crop(EntityId id, const Position& pos, const PlantDefinition& definition, uint32_t ticksPerDay):id(id),pos(pos),definition(definition),quality(ticksPerDay, definition.wateringInterval),targetGrowthTicks(definition.ticksToGrow),toleratedDryTicks(uint64_t{definition.toleratedMissedWaterings} * definition.wateringInterval) {
    if (id == 0 || pos.getX() < 0 || pos.getY() < 0 || ticksPerDay == 0 ||
        definition.ticksToGrow == 0 || definition.wateringInterval == 0) {
        throw std::invalid_argument("Identificador, posicion o tiempos de cultivo invalidos");
    }
    getProduct();
}

ItemId Crop::getProduct() const {
    switch (definition.type) {
        case PlantType::Strawberry: return ItemId::Strawberry;
        case PlantType::Scallion: return ItemId::Scallion;
        case PlantType::Potato: return ItemId::Potato;
        case PlantType::Pumpkin: return ItemId::Pumpkin;
    }
    throw std::invalid_argument("Tipo de cultivo desconocido");
}

void Crop::tick() {
    if (harvested) {
        return;
    }
    if (isReady()) {
        quality.waitForHarvest();
        return;
    }
    if (state == CropState::NeedsWater) {
        if (toleratedDryTicks == 0) {
            quality.waitForWater();
            return;
        }
        --toleratedDryTicks;
    } else {
        ++ticksSinceWater;
    }
    ++growthTicks;
    if (growthTicks == targetGrowthTicks) {
        state = CropState::Ready;
    } else if (state == CropState::Growing && ticksSinceWater >= definition.wateringInterval) {
        state = CropState::NeedsWater;
        quality.requestWater();
    }
}

bool Crop::water() {
    if (harvested || state != CropState::NeedsWater) {
        return false;
    }
    state = CropState::Growing;
    ticksSinceWater = 0;
    return true;
}

std::optional<FarmProduct> Crop::getHarvest(double luck) const {
    const auto result = quality.evaluate(luck);
    if (!isReady()) {
        return std::nullopt;
    }
    return FarmProduct{getProduct(), 1, result};
}

void Crop::resetCycle() {
    state = CropState::NeedsWater;
    growthTicks = 0;
    targetGrowthTicks = definition.regrowthTicks;
    ticksSinceWater = 0;
    quality.reset();
    toleratedDryTicks = uint64_t{definition.toleratedMissedWaterings} * definition.wateringInterval;
}

std::optional<FarmProduct> Crop::harvest(double luck) {
    auto product = getHarvest(luck);
    if (product) {
        if (definition.regrowthTicks > 0) {
            resetCycle();
        } else {
            harvested = true;
        }
    }
    return product;
}