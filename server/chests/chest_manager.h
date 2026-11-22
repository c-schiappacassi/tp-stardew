#pragma once

#include <map>

#include "commands/chest_commands.h"
#include "farm/ports/farm_placement.h"

#include "chest.h"

class ChestManager {
private:
    std::map<EntityId, Chest> chests;
    ChestCommands commands;

public:
    bool place(EntityId id, const Position& position, FarmPlacement& placement, const std::map<uint8_t, Stack>& items = {});
    bool removeEmpty(EntityId id, FarmPlacement& placement);
    const Chest* find(EntityId id) const;
    std::optional<ChestSnapshot> snapshot(EntityId id) const;
    bool apply(EntityId id, PlayerManager& players, uint16_t playerId, const ChestAction& action);
    const std::map<EntityId, Chest>& getChests() const;
};
