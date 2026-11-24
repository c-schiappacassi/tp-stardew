#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "dtos.h"
#include "community_save_data.h"

struct PersistenceDTO {
    std::string id;
    std::string farmName;
    uint8_t mapId = 0;
    uint32_t currentDay = 1;

    std::vector<std::string> users;
    std::map<std::string, PlayerDataLocal> players;

    FarmData farm;
    std::vector<InteriorData> interiors;
    std::map<uint8_t, std::vector<FurnitureData>> dungeonFurniture;
    std::map<uint8_t, std::vector<LootData>> dungeonStorageLoot;
    std::map<uint32_t, std::map<uint8_t, Stack>> chestInventories;
    std::vector<std::pair<Position, uint64_t>> emptySoilElapsed;

    std::map<uint32_t, uint64_t> cropGrowthProgress;
    std::map<uint32_t, uint64_t> cropWaterRemainingTime;
    std::map<uint32_t, bool> harvestedCrops;
    std::map<uint32_t, bool> regrowingCrops;

    std::map<uint32_t, uint64_t> animalProductionProgress;
    std::map<uint32_t, uint32_t> animalAvailableProducts;

    CommunitySaveData communityCenterActionProgress;
};
