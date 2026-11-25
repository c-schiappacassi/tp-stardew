#include "dungeon_persistence.h"

#include "server_constants.h"

#include <stdexcept>
#include <utility>

#include "dungeon/rooms/dungeon_rooms.h"

std::map<uint8_t, std::vector<FurnitureData>> DungeonPersistence::storageFurniture(const DungeonRooms& rooms) const {
    std::map<uint8_t, std::vector<FurnitureData>> result;
    for (const auto& [number, room]: rooms) {
        if (room.getPassages())
            result.emplace(number, room.getChests().furnitureData());
    }
    return result;
}

std::map<uint8_t, std::vector<LootData>> DungeonPersistence::storageLoot(const DungeonRooms& rooms) const {
    std::map<uint8_t, std::vector<LootData>> result;
    for (const auto& [number, room]: rooms) {
        if (room.getPassages())
            result.emplace(number, room.getStorageItems().getPublicData());
    }
    return result;
}

void DungeonPersistence::saveInventories(const DungeonRooms& rooms, std::map<uint32_t, std::map<uint8_t, Stack>>& inventories) const {
    for (const auto& [number, room]: rooms) {
        auto contents = room.getChests().inventories();
        inventories.merge(contents);
    }
}

void DungeonPersistence::restoreStorage(DungeonRooms& rooms, const std::map<uint8_t, std::vector<FurnitureData>>& saved,
    const std::map<uint32_t, std::map<uint8_t, Stack>>& inventories, const std::map<uint8_t, std::vector<LootData>>& loot) const {
    auto prepared = rooms;
    for (const auto& [number, furniture]: saved) {
        auto* room = prepared.find(number);
        if (!room || !room->getPassages())
            throw std::runtime_error(server_constants::INVALID_SAVED_DUNGEON_STORAGE);
        const auto dropped = loot.find(number);
        room->restoreChests(furniture, inventories, dropped == loot.end() ? std::vector<LootData>{} : dropped->second);
    }
    rooms = std::move(prepared);
}
