#pragma once

#include <map>
#include <optional>
#include "decoration.h"
#include "farm/ports/farm_placement.h"

class DecorationManager {
private:
    std::map<EntityId, Decoration> decorations;

public:
    bool place(EntityId id, const Position& position, ItemId item, FarmPlacement& placement);
    std::optional<ItemId> remove(EntityId id, FarmPlacement& placement);
    const Decoration* find(EntityId id) const;
    const std::map<EntityId, Decoration>& getDecorations() const;
};
