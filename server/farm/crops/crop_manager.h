#pragma once

#include <map>
#include <set>
#include <vector>
#include "crop.h"
#include "farm/ports/farm_placement.h"

class FarmConfig;

class CropManager {
private:
    std::vector<CropDefinition> definitions;
    std::chrono::milliseconds dayLength;
    std::map<EntityId, Crop> crops;
    std::set<ZoneMap::Cell> tilledSoil;

    const CropDefinition* findDefinition(CropType type) const;

public:
    explicit CropManager(const FarmConfig& config);
    bool hasTilledSoil() const;
    bool tillSoil(const Position& position, const FarmPlacement& placement);
    bool isTilled(const Position& position, const FarmPlacement& placement) const;
    bool plant(EntityId id, const Position& position, CropType type, FarmPlacement& placement);
    bool water(EntityId id);
    std::optional<FarmProduct> harvest(EntityId id, FarmPlacement& placement, double luck = 1.0);
    void update(std::chrono::milliseconds elapsed);
    void finishDay();
    Crop* findMutable(EntityId id);
    const Crop* find(EntityId id) const;
    const std::map<EntityId, Crop>& getCrops() const;
    const std::set<ZoneMap::Cell>& getTilledSoil() const;
};
