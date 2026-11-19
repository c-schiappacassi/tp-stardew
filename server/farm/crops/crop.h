#pragma once

#include <chrono>
#include <optional>
#include "dtos.h"
#include "entities/entity_id.h"
#include "crop_definition.h"
#include "farm/farm_product.h"
#include "crop_quality.h"

class Crop {
private:
    EntityId id;
    Position position;
    CropDefinition definition;
    CropQuality quality;
    std::chrono::milliseconds growthElapsed{0};
    std::chrono::milliseconds growthDuration;
    std::chrono::milliseconds remainingWaterTime{0};
    std::chrono::milliseconds remainingDryTolerance;
    bool harvested = false;

    ItemId getProduct() const;
    void resetCycle();

public:
    Crop(EntityId id, const Position& position, const CropDefinition& definition,
        std::chrono::milliseconds dayLength);
    EntityId getId() const;
    const Position& getPosition() const;
    CropType getType() const;
    CropState getState() const;
    bool isReady() const;
    bool isHarvested() const;
    bool isWatered() const;
    std::chrono::milliseconds getGrowthElapsed() const;
    std::chrono::milliseconds getRemainingWaterTime() const;
    CropData getPublicData() const;
    void update(std::chrono::milliseconds elapsed);
    void finishDay();
    bool water();
    std::optional<FarmProduct> getHarvest(double luck = 1.0) const;
    std::optional<FarmProduct> harvest(double luck = 1.0);
};
