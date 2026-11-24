#include "chest.h"

#include <limits>
#include <stdexcept>

Chest::Chest(EntityId id, const Position& position, const std::map<uint8_t, Stack>& items): id(id), position(position) {
    if (id == 0 || position.getX() < 0 || position.getY() <= 0 || position.getX() > std::numeric_limits<uint16_t>::max() ||
        position.getY() > std::numeric_limits<uint16_t>::max()) {
        throw std::invalid_argument("Invalid chest identifier or position");
    }
    inventory.restoreSlots(items);
}

EntityId Chest::getId() const { return id; }
const Position& Chest::getPosition() const { return position; }
bool Chest::isEmpty() const { return inventory.getItems().empty(); }
const Inventory& Chest::getInventory() const { return inventory; }
FurnitureData Chest::getPublicData() const { return {id, position, FurnitureId::Chest, Direction::Down}; }
ChestSnapshot Chest::snapshot() const { return ChestSnapshot{*this}; }
