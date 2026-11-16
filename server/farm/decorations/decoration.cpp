#include "decoration.h"

#include <stdexcept>

Decoration::Decoration(EntityId id, const Position& position, ItemId item):
    id(id), position(position), item(item) {
    if (id == 0 || position.getX() < 0 || position.getY() < 0 || !isDecoration(item)) {
        throw std::invalid_argument("Identificador, posicion u objeto decorativo invalidos");
    }
}

bool Decoration::isDecoration(ItemId item) {
    return item >= ItemId::Chair && item <= ItemId::BrownArmCouch &&
           item != ItemId::Bed && item != ItemId::BedSheet;
}

EntityId Decoration::getId() const { return id; }
const Position& Decoration::getPosition() const { return position; }
ItemId Decoration::getItem() const { return item; }
bool Decoration::blocksMovement() const {
    return item != ItemId::BeigeRug && item != ItemId::BlueRug;
}
