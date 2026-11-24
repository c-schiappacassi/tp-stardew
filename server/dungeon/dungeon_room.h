#pragma once

#include <cstdint>
#include <map>
#include <optional>

#include "chests/chest_manager.h"
#include "chests/loading/chest_spawn.h"
#include "combat/room_combat.h"
#include "contracts/dungeon_room_passages.h"
#include "entities/placement/placement_space.h"
#include "items/ground_items.h"
#include "rooms/room_geometry.h"
#include "rooms/room_minerals.h"

class DungeonRoom {
private:
    uint8_t number;
    uint8_t visitDepth;
    PlacementSpace space;
    RoomMinerals minerals;
    RoomGeometry geometry;
    std::optional<DungeonRoomPassages> passages;
    RoomCombat combat;
    GroundItems groundItems;
    ChestManager chests;
    GroundItems storageItems;

    void clearChestOverlaps(const std::vector<FurnitureData>& furniture);

public:
    explicit DungeonRoom(uint8_t number);
    bool initialize(const ZoneMap& map, const DungeonRoomPassages& data, const std::vector<Position>& reserved, const std::vector<ChestSpawn>& initial, EntityIdGenerator& ids);
    uint8_t getNumber() const;
    uint8_t getVisitDepth() const;
    void setVisitDepth(uint8_t depth);
    bool addMineral(Mineral mineral);
    bool removeMineral(EntityId id);
    Mineral* findMineral(EntityId id);
    const Mineral* findMineral(EntityId id) const;
    const std::map<EntityId, Mineral>& getMinerals() const;
    const ZoneMap& getMap() const;
    bool canWalk(const CollisionBounds& bounds) const;
    void removeBrokenMinerals();
    const std::optional<DungeonRoomPassages>& getPassages() const;
    bool hasClearLine(const Position& from, const Position& target) const;
    std::optional<MineData> getMineData(bool includeBroken = false) const;
    RoomCombat& getCombat();
    const RoomCombat& getCombat() const;
    DungeonCombatData getCombatData() const;
    GroundItems& getGroundItems();
    const GroundItems& getGroundItems() const;
    ChestManager& getChests();
    const ChestManager& getChests() const;
    GroundItems& getStorageItems();
    const GroundItems& getStorageItems() const;
    std::optional<bool> interactWithChest(PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now);
    void restoreChests(const std::vector<FurnitureData>& furniture, const std::map<uint32_t, std::map<uint8_t, Stack>>& inventories, const std::vector<LootData>& loot);
};
