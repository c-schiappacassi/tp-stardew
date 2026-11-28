#include "decoration.h"

#include "server_constants.h"

#include <stdexcept>

#include "decoration_items.h"
#include "world/interiors/interior_furniture_position.h"

Decoration::Decoration(EntityId id, const Position& position, ItemId item, Direction direction):
    id(id), position(position), item(item), direction(direction) {
    if (id == 0 || position.getX() < 0 || position.getY() < 0 || !isDecoration(item) ||
        direction < Direction::Up || direction > Direction::Right) {
        throw std::invalid_argument(server_constants::INVALID_DECORATION_IDENTIFIER_POSITION_OR_ITEM);
    }
}

bool Decoration::isDecoration(ItemId item) {
    return decoration_items::isDecoration(item);
}

EntityId Decoration::getId() const { return id; }
const Position& Decoration::getPosition() const { return position; }
ItemId Decoration::getItem() const { return item; }
Direction Decoration::getDirection() const { return direction; }
CollisionBox Decoration::collisionBoxFor(ItemId item, Direction direction) {
    return furnitureCollisionBox(decoration_items::furnitureFor(item), direction);
}

CollisionBounds Decoration::getBounds() const { return collisionBoxFor(item, direction).at(position); }

bool Decoration::blocksMovement() const {
    return item != ItemId::BeigeRug && item != ItemId::BlueRug;
}

FurnitureData Decoration::getPublicData() const {
    return FurnitureData{id, position, decoration_items::furnitureFor(item), direction};
}
