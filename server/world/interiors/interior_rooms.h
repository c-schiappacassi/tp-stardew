#pragma once

#include <vector>

#include "interior.h"

class InteriorRooms {
private:
    std::vector<Interior> rooms;

    Interior* find(const InteriorLocation& location);
    Interior* roomFor(const PlayerSnapshot& player);
    const Interior* roomFor(const PlayerSnapshot& player) const;

public:
    using const_iterator = std::vector<Interior>::const_iterator;

    bool initialize(const std::vector<InteriorLayout>& layouts, EntityIdGenerator& ids);
    const Interior* find(const InteriorLocation& location) const;
    const_iterator begin() const;
    const_iterator end() const;
    bool canWalk(const InteriorLocation& location, const CollisionBounds& bounds) const;
    bool canMove(const PlayerSnapshot& player, const Position& next) const;
    bool useBed(PlayerManager& players, uint16_t playerId) const;
    std::optional<WorldLocation> recoveryFor(const PlayerSnapshot& player, uint16_t playerId) const;
    std::optional<NpcData> nearbyMerchant(const PlayerSnapshot& player, Direction direction) const;
    InteriorData dataFor(const InteriorLocation& location) const;
    void completeMap(ConstMapData& map) const;
    bool useSelectedItem(PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now);
    std::optional<OpenChestData> openChestFor(const PlayerSnapshot& player) const;
    void depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command);
    void withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command);
    void collectNearbyItems(PlayerManager& players, uint16_t playerId);
    std::vector<InteriorData> storageData() const;
    void saveInventories(std::map<uint32_t, std::map<uint8_t, Stack>>& inventories) const;
    void restoreStorage(const std::vector<InteriorData>& saved, const std::map<uint32_t, std::map<uint8_t, Stack>>& inventories);
};
