#include "decoration_manager.h"
#include "farm/spatial/farm_reservation.h"

const std::map<EntityId, Decoration>& DecorationManager::getDecorations() const {
    return decorations;
}

bool DecorationManager::place(EntityId id, const Position& position, ItemId item,
    FarmPlacement& placement, Direction direction) {
    if (!Decoration::isDecoration(item) || id == 0 || placement.contains(id) ||
        direction < Direction::Up || direction > Direction::Right ||
        !placement.isAvailable(CollisionBox{entity_dimensions::DECORATION}.at(position))) {
        return false;
    }
    const Decoration decoration{id, position, item, direction};
    const CollisionBox collision{entity_dimensions::DECORATION};
    FarmReservation reservation{placement, id, position, collision, decoration.blocksMovement()};
    if (!reservation.isReserved() || !decorations.emplace(id, decoration).second) {
        return false;
    }
    reservation.commit();
    return true;
}

std::optional<ItemId> DecorationManager::remove(EntityId id, FarmPlacement& placement) {
    const auto it = decorations.find(id);
    if (it == decorations.end()) {
        return std::nullopt;
    }
    const auto item = it->second.getItem();
    placement.release(id);
    decorations.erase(it);
    return item;
}

const Decoration* DecorationManager::find(EntityId id) const {
    const auto it = decorations.find(id);
    if (it == decorations.end()) {
        return nullptr;
    } else {
        return &it->second;
    }
}
