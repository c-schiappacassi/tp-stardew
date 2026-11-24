#include "decoration.h"

#include <stdexcept>

#include "decoration_items.h"

Decoration::Decoration(EntityId id, const Position& position, ItemId item, Direction direction):
    id(id), position(position), item(item), direction(direction) {
    if (id == 0 || position.getX() < 0 || position.getY() < 0 || !isDecoration(item) ||
        direction < Direction::Up || direction > Direction::Right) {
        throw std::invalid_argument("Invalid decoration identifier, position or item");
    }
}

bool Decoration::isDecoration(ItemId item) {
    return decoration_items::isDecoration(item);
}

EntityId Decoration::getId() const { return id; }
const Position& Decoration::getPosition() const { return position; }
ItemId Decoration::getItem() const { return item; }
Direction Decoration::getDirection() const { return direction; }
bool Decoration::blocksMovement() const {
    return item != ItemId::BeigeRug && item != ItemId::BlueRug;
}

FurnitureData Decoration::getPublicData() const {
    return FurnitureData{id, position, decoration_items::furnitureFor(item), direction};
}
