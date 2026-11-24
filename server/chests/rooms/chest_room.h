#pragma once

#include <chrono>

#include "chests/chest_manager.h"
#include "chests/loading/chest_spawn.h"
#include "entities/placement/placement_space.h"
#include "items/ground_items.h"

class ChestRoom {
private:
    ChestManager chests;
    PlacementSpace space;
    std::vector<CollisionBounds> fixedObstacles;
    GroundItems groundItems;

public:
    bool initialize(const ZoneMap& terrain, const std::vector<Position>& reserved, const std::vector<CollisionBounds>& obstacles, const std::vector<ChestSpawn>& initial,
        EntityIdGenerator& ids);
    bool canWalk(const CollisionBounds& bounds) const;
    std::vector<FurnitureData> furniture() const;
    std::optional<bool> interact(PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now,
        const std::vector<CollisionBounds>& obstacles = {});
    bool deposit(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command);
    bool withdraw(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command);
    std::optional<OpenChestData> contentsFor(const PlayerSnapshot& player) const;
    const std::map<EntityId, Chest>& getChests() const;
    GroundItems& getGroundItems();
    const GroundItems& getGroundItems() const;
    bool collectNearbyItems(PlayerManager& players, uint16_t playerId);
    void restore(const std::vector<FurnitureData>& furniture, const std::map<uint32_t, std::map<uint8_t, Stack>>& inventories, const std::vector<LootData>& loot = {});
};
