#pragma once

#include <map>
#include <optional>
#include "decoration.h"
#include "entities/placement/placement.h"

class DecorationManager {
private:
    std::map<EntityId, Decoration> decorations;

public:
    bool place(EntityId id, const Position& position, ItemId item, Placement& placement,Direction direction = Direction::Down);
    std::optional<ItemId> remove(EntityId id, Placement& placement);
    const Decoration* find(EntityId id) const;
    std::optional<FurnitureData> dataFor(EntityId id) const;
    const std::map<EntityId, Decoration>& getDecorations() const;
};
