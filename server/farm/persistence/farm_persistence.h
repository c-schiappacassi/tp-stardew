#pragma once

#include "farm_save_data.h"

class FarmConfig;
class FarmTerrain;
class CropManager;
class AnimalManager;
class DecorationManager;
class ChestManager;
class GroundItems;

class FarmPersistence {
public:
    static FarmSaveData save(const FarmTerrain& terrain, const CropManager& crops, const AnimalManager& animals, const DecorationManager& decorations,
        const ChestManager& chests, const GroundItems& groundItems);
    static void restore(const FarmRestoreData& saved, const FarmConfig& config, uint32_t seed, FarmTerrain& terrain, CropManager& crops, AnimalManager& animals,
        DecorationManager& decorations, ChestManager& chests, GroundItems& groundItems);
};
