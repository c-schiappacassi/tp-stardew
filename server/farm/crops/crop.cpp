#include "crop.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

Crop::Crop(EntityId id, const Position& position, const CropDefinition& definition,
    std::chrono::milliseconds dayLength):
    id(id), position(position), definition(definition),
    quality(dayLength, definition.wateringInterval), growthDuration(definition.growthTime),
    remainingDryTolerance(0) {
    const bool invalidTolerance = definition.toleratedMissedWaterings != 0 &&
        definition.wateringInterval.count() > std::numeric_limits<int64_t>::max() / definition.toleratedMissedWaterings;
    if (id == 0 || position.getX() < 0 || position.getY() <= 0 ||
        definition.growthTime.count() <= 0 || definition.regrowthTime.count() < 0 ||invalidTolerance) {
        throw std::invalid_argument("Invalid crop identifier, position or times");
    }
    remainingDryTolerance = definition.wateringInterval * definition.toleratedMissedWaterings;
    getProduct();
}

EntityId Crop::getId() const { return id; }
const Position& Crop::getPosition() const { return position; }
CropType Crop::getType() const { return definition.type; }
bool Crop::isReady() const { return !harvested && growthElapsed >= growthDuration; }
bool Crop::isHarvested() const { return harvested; }
bool Crop::isRegrowing() const { return regrowing; }
bool Crop::isWatered() const { return remainingWaterTime.count() > 0 && !isReady() && !harvested; }
std::chrono::milliseconds Crop::getGrowthElapsed() const { return growthElapsed; }
std::chrono::milliseconds Crop::getRemainingWaterTime() const { return remainingWaterTime; }

CropState Crop::getState() const {
    if (isReady()) {
        return CropState::Ready;
    }
    if (regrowing) {
        return CropState::Growing;
    }
    const double progress = static_cast<double>(growthElapsed.count()) / growthDuration.count();
    if (progress < 1.0 / 3.0) {
        return CropState::JustPlanted;
    } else if (progress < 2.0 / 3.0) {
        return CropState::Sprouting;
    } else {
        return CropState::Growing;
    }
}

CropData Crop::getPublicData() const {
    return CropData{id, position, getType(), getState(), isWatered()};
}

ItemId Crop::getProduct() const {
    const auto item = farm_products::cropItem(definition.type);
    if (!item)
        throw std::invalid_argument("Unknown crop type");
    return *item;
}

void Crop::update(std::chrono::milliseconds elapsed) {
    if (elapsed.count() < 0) {
        throw std::invalid_argument("Negative time");
    }
    if (harvested) {
        return;
    }
    while (elapsed.count() > 0) {
        if (isReady()) {
            quality.addHarvestDelay(elapsed);
            return;
        }
        const bool watered = remainingWaterTime.count() > 0;
        const auto availableGrowthTime = watered ? remainingWaterTime : remainingDryTolerance;
        if (availableGrowthTime.count() == 0) {
            quality.addWateringDelay(elapsed);
            return;
        }
        const auto remainingGrowth = growthDuration - growthElapsed;
        const auto growthStep = std::min({elapsed, availableGrowthTime, remainingGrowth});
        if (watered) {
            remainingWaterTime -= growthStep;
        } else {
            remainingDryTolerance -= growthStep;
        }
        growthElapsed += growthStep;
        elapsed -= growthStep;
        if (watered && remainingWaterTime.count() == 0 && !isReady()) {
            quality.recordWaterRequest();
        }
    }
}

void Crop::finishDay() {
    if (isWatered()) quality.recordWaterRequest();
    remainingWaterTime = std::chrono::milliseconds{0};
}

bool Crop::water() {
    if (harvested || isReady() || isWatered()) {
        return false;
    }
    remainingWaterTime = definition.wateringInterval;
    return true;
}

void Crop::growFully() {
    if (harvested) return;
    growthElapsed = growthDuration;
    remainingWaterTime = std::chrono::milliseconds{0};
    quality.reset();
}

std::optional<FarmProduct> Crop::getHarvest(double luck) const {
    const auto result = quality.evaluate(luck);
    if (!isReady()) {
        return std::nullopt;
    }
    return FarmProduct{getProduct(), 1, result};
}

void Crop::resetCycle() {
    regrowing = true;
    growthElapsed = std::chrono::milliseconds{0};
    growthDuration = definition.regrowthTime;
    remainingWaterTime = std::chrono::milliseconds{0};
    remainingDryTolerance = definition.wateringInterval * definition.toleratedMissedWaterings;
    quality.reset();
}

std::optional<FarmProduct> Crop::harvest(double luck) {
    auto product = getHarvest(luck);
    if (product) {
        if (definition.regrowthTime.count() > 0) {
            resetCycle();
        } else {
            harvested = true;
        }
    }
    return product;
}

void Crop::restoreProgress(std::chrono::milliseconds growth, std::chrono::milliseconds water,
    bool wasHarvested, bool wasRegrowing) {
    const auto restoredDuration = wasRegrowing ? definition.regrowthTime : definition.growthTime;
    if (restoredDuration.count() <= 0 || growth.count() < 0 || water.count() < 0 ||
        growth > restoredDuration ||
        water > definition.wateringInterval) {
        throw std::invalid_argument("Invalid saved crop progress");
    }
    regrowing = wasRegrowing;
    growthDuration = restoredDuration;
    growthElapsed = growth;
    remainingWaterTime = water;
    harvested = wasHarvested;
}
