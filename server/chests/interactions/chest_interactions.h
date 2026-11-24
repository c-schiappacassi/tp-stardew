#pragma once

#include <chrono>
#include <optional>

#include "dtos.h"

class ChestManager;
class ChestSnapshot;
class EntityIdGenerator;
class Placement;
class GroundItems;
class PlayerManager;
class PlayerSnapshot;
class ZoneMap;

class ChestInteractions {
private:
    bool canAccess(const PlayerSnapshot& player, const ChestSnapshot& chest, const ZoneMap& terrain) const;
    std::optional<ChestSnapshot> findTarget(const PlayerSnapshot& player, const ChestManager& chests, const ZoneMap& terrain, Direction direction) const;

public:
    std::optional<bool> interact(PlayerManager& players, uint16_t playerId, ChestManager& chests, Placement& placement, const ZoneMap& terrain, GroundItems& groundItems,
        Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) const;
    bool deposit(PlayerManager& players, uint16_t playerId, ChestManager& chests, const ZoneMap& terrain, const ItemTransferDTO& command) const;
    bool withdraw(PlayerManager& players, uint16_t playerId, ChestManager& chests, const ZoneMap& terrain, const SelectItemDTO& command) const;
    std::optional<OpenChestData> contentsFor(const PlayerSnapshot& player, const ChestManager& chests, const ZoneMap& terrain) const;
};
