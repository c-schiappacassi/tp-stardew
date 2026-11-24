#pragma once

#include <cstdint>
#include <map>
#include <utility>
#include <vector>

#include "dtos.h"

struct FarmRestoreData {
    const FarmData& farm;
    const std::map<uint32_t, uint64_t>& cropGrowthProgress;
    const std::map<uint32_t, uint64_t>& cropWaterRemainingTime;
    const std::map<uint32_t, bool>& harvestedCrops;
    const std::map<uint32_t, bool>& regrowingCrops;
    const std::map<uint32_t, uint64_t>& animalProductionProgress;
    const std::map<uint32_t, uint32_t>& animalAvailableProducts;
    const std::vector<std::pair<Position, uint64_t>>& emptySoilElapsed;
    const std::map<uint32_t, std::map<uint8_t, Stack>>& chestInventories;
};

struct FarmSaveData {
    FarmData farm;
    std::map<uint32_t, uint64_t> cropGrowthProgress;
    std::map<uint32_t, uint64_t> cropWaterRemainingTime;
    std::map<uint32_t, bool> harvestedCrops;
    std::map<uint32_t, bool> regrowingCrops;
    std::map<uint32_t, uint64_t> animalProductionProgress;
    std::map<uint32_t, uint32_t> animalAvailableProducts;
    std::vector<std::pair<Position, uint64_t>> emptySoilElapsed;
    std::map<uint32_t, std::map<uint8_t, Stack>> chestInventories;

    FarmRestoreData view() const;
};
