#pragma once

#include <cstdint>
#include <map>
#include <optional>

#include "chests/rooms/chest_room.h"
#include "dungeon/combat/room_combat.h"
#include "items/ground_items.h"
#include "world/zone_map.h"

#include "dungeon_room_passages.h"
#include "room_geometry.h"
#include "room_minerals.h"


class DungeonRoom {
private:
    uint8_t number;
    ZoneMap terrain;
    RoomMinerals minerals;
    RoomGeometry geometry;
    std::optional<DungeonRoomPassages> passages;
    RoomCombat combat;
    GroundItems groundItems;
    ChestRoom storage;

public:
    explicit DungeonRoom(uint8_t number);

    uint8_t getNumber() const;
    bool addMineral(Mineral mineral);
    bool removeMineral(EntityId id);
    Mineral* findMineral(EntityId id);
    const Mineral* findMineral(EntityId id) const;
    const std::map<EntityId, Mineral>& getMinerals() const;
    bool setMap(const ZoneMap& map);
    const ZoneMap& getMap() const;
    bool canWalk(const CollisionBounds& bounds) const;
    void removeBrokenMinerals();
    bool setPassages(const DungeonRoomPassages& data);
    const std::optional<DungeonRoomPassages>& getPassages() const;
    bool hasClearLine(const Position& from,const Position& target) const;
    std::optional<MineData> getMineData(bool includeBroken = false) const;
    RoomCombat& getCombat();
    const RoomCombat& getCombat() const;
    DungeonCombatData getCombatData() const;
    GroundItems& getGroundItems();
    const GroundItems& getGroundItems() const;
    bool initializeChests(const std::vector<ChestSpawn>& initial, const std::vector<Position>& reserved, EntityIdGenerator& ids);
    std::optional<bool> interactWithChest(PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now);
    void restoreChests(const std::vector<FurnitureData>& furniture, const std::map<uint32_t, std::map<uint8_t, Stack>>& inventories, const std::vector<LootData>& loot);
    ChestRoom& getStorage();
    const ChestRoom& getStorage() const;
};
