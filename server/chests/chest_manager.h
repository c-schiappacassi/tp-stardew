#pragma once

#include <chrono>
#include <map>
#include <optional>
#include <vector>

#include "commands/chest_commands.h"
#include "entities/placement/placement.h"

#include "chest.h"

class EntityIdGenerator;
class GroundItems;
class PlayerSnapshot;

class ChestManager {
private:
    std::map<EntityId, Chest> chests;
    ChestCommands commands;

public:
    bool place(EntityId id, const Position& position, Placement& placement, const std::map<uint8_t, Stack>& items = {});
    bool removeEmpty(EntityId id, Placement& placement);
    const Chest* find(EntityId id) const;
    std::optional<ChestSnapshot> snapshot(EntityId id) const;
    bool apply(EntityId id, PlayerManager& players, uint16_t playerId, const ChestAction& action);
    std::optional<bool> interact(PlayerManager& players, uint16_t playerId, Placement& placement, const ZoneMap& terrain, GroundItems& groundItems,Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now);
    std::optional<OpenChestData> contentsFor(const PlayerSnapshot& player, const ZoneMap& terrain) const;
    bool deposit(PlayerManager& players, uint16_t playerId, const ZoneMap& terrain, const ItemTransferDTO& command);
    bool withdraw(PlayerManager& players, uint16_t playerId, const ZoneMap& terrain, const SelectItemDTO& command);
    const std::map<EntityId, Chest>& getChests() const;
    std::vector<FurnitureData> furnitureData() const;
    std::map<uint32_t, std::map<uint8_t, Stack>> inventories() const;
};
