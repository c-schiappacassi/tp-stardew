#pragma once

#include <map>
#include <vector>

#include "dtos.h"

class DungeonRooms;

class DungeonPersistence {
public:
    std::map<uint8_t, std::vector<FurnitureData>> storageFurniture(const DungeonRooms& rooms) const;
    std::map<uint8_t, std::vector<LootData>> storageLoot(const DungeonRooms& rooms) const;
    void saveInventories(const DungeonRooms& rooms, std::map<uint32_t, std::map<uint8_t, Stack>>& inventories) const;
    void restoreStorage(DungeonRooms& rooms, const std::map<uint8_t, std::vector<FurnitureData>>& saved, const std::map<uint32_t, std::map<uint8_t, Stack>>& inventories,
        const std::map<uint8_t, std::vector<LootData>>& loot) const;
};
