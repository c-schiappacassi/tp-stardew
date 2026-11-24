#pragma once

#include "chests/rooms/chest_room.h"

#include "interior_layout.h"

class PlayerSnapshot;
class PlayerManager;

class InteriorManager {
    std::vector<InteriorLayout> rooms;
    std::map<std::pair<InteriorId, uint8_t>, ChestRoom> storage;

public:
    bool initialize(const std::vector<InteriorLayout>& layouts, EntityIdGenerator& ids);
    const InteriorLayout* find(const InteriorLocation& location) const;
    const std::vector<InteriorLayout>& layouts() const { return rooms; }
    bool canWalk(const InteriorLocation& location, const CollisionBounds& bounds) const;
    bool useBed(PlayerManager& players, uint16_t playerId) const;
    std::optional<WorldLocation> recoveryFor(const PlayerSnapshot& player, uint16_t playerId) const;
    InteriorData dataFor(const InteriorLocation& location) const;
    void completeMap(ConstMapData& map) const;
    bool interactWithChest(PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now);
    std::optional<OpenChestData> openChestFor(const PlayerSnapshot& player) const;
    void depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command);
    void withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command);
    void collectNearbyItems(PlayerManager& players, uint16_t playerId);
    std::vector<InteriorData> storageData() const;
    void saveInventories(std::map<uint32_t, std::map<uint8_t, Stack>>& inventories) const;
    void restoreStorage(const std::vector<InteriorData>& saved, const std::map<uint32_t, std::map<uint8_t, Stack>>& inventories);
};
