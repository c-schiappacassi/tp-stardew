#include "farm_save_data.h"

FarmRestoreData FarmSaveData::view() const {
    return FarmRestoreData{farm, cropGrowthProgress, cropWaterRemainingTime, harvestedCrops, regrowingCrops,
        animalProductionProgress, animalAvailableProducts, emptySoilElapsed, chestInventories};
}
