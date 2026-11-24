#pragma once

#include <map>
#include <set>
#include <vector>
#include "crop.h"
#include "entities/placement/placement.h"

class FarmConfig;
class FarmTerrain;

class CropManager {
private:
    std::vector<CropDefinition> definitions;
    std::chrono::milliseconds dayLength;
    std::map<EntityId, Crop> crops;
    std::set<ZoneMap::Cell> tilledSoil;
    std::map<ZoneMap::Cell, std::chrono::milliseconds> emptySoilElapsed;

    const CropDefinition* findDefinition(CropType type) const;

public:
    explicit CropManager(const FarmConfig& config);
    bool hasTilledSoil() const;
    bool tillSoil(const Position& position, const FarmTerrain& terrain);
    bool isTilled(const Position& position, const Placement& placement) const;
    bool plant(EntityId id, const Position& position, CropType type, Placement& placement);
    bool water(EntityId id);
    std::optional<FarmProduct> harvest(EntityId id, Placement& placement, double luck = 1.0);
    void update(std::chrono::milliseconds elapsed);
    void finishDay();
    std::size_t care();
    std::vector<Position> tilledPositions(const ZoneMap& map) const;
    std::vector<std::pair<Position, uint64_t>> emptySoilElapsedData(const ZoneMap& map) const;
    Crop* findMutable(EntityId id);
    const Crop* find(EntityId id) const;
    std::optional<CropData> dataFor(EntityId id) const;
    const std::map<EntityId, Crop>& getCrops() const;
    const std::set<ZoneMap::Cell>& getTilledSoil() const;
    const std::map<ZoneMap::Cell, std::chrono::milliseconds>& getEmptySoilElapsed() const;
    bool restoreEmptySoilElapsed(const Position& position, std::chrono::milliseconds elapsed,const Placement& placement);
};
