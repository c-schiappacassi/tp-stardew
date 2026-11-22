#include "player.h"

#include <utility>

const std::map<uint8_t, Stack>& Player::getInventoryItems() const { return inventory.getItems(); }

bool Player::selectSlot(uint8_t slot) {
    if (!canAct()) {
        return false;
    }
    if (slot >= inventory.getCapacity()) {
        return false;
    }

    currentSlot = slot;
    return true;
}

bool Player::addItem(const Stack& stack) { return inventory.add(stack); }

uint16_t Player::addItemUpTo(const Stack& stack) { return inventory.addUpTo(stack); }

bool Player::removeItem(uint8_t slot, uint16_t quantity) {
    return inventory.remove(slot, quantity);
}

const Stack* Player::getSelectedItem() const { return inventory.getSlot(currentSlot); }

uint8_t Player::getSelectedSlot() const { return currentSlot; }

bool Player::removeItems(const std::vector<std::pair<uint8_t, uint16_t>>& quantities) {
    if (!canAct() || quantities.empty()) return false;
    Inventory prepared = inventory;
    for (const auto& [slot, quantity] : quantities) {
        if (!prepared.remove(slot, quantity)) return false;
    }
    inventory = std::move(prepared);
    return true;
}
