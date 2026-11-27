#include "inventory.h"

#include "server_constants.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

bool Inventory::validStack(const Stack& stack) {
    const auto item = stack.item_id;
    const bool knownItem = (item >= ItemId::Pickaxe && item <= ItemId::Sword) || (item >= ItemId::StrawberrySeed && item <= ItemId::PumpkinSeed) ||
        (item >= ItemId::Strawberry && item <= ItemId::Pumpkin) || (item >= ItemId::Coal && item <= ItemId::Gold) || (item >= ItemId::Milk && item <= ItemId::Egg) ||
        (item >= ItemId::Chest && item <= ItemId::BrownArmCouch);
    return knownItem && stack.quantity != 0 && stack.qual <= Quality::Perfect;
}

bool Inventory::isTool(ItemId item) {
    return item == ItemId::Pickaxe || item == ItemId::WateringCan ||
        item == ItemId::Shovel || item == ItemId::Sword;
}

uint16_t Inventory::getStackLimit(ItemId item) const {
    return isTool(item) ? 1 : stackLimit;
}

Inventory::Inventory(uint8_t capacity, uint16_t stackLimit):capacity(capacity), stackLimit(stackLimit) {
    if (capacity == 0 || stackLimit == 0) {
        throw std::invalid_argument(
            server_constants::CAPACITY_AND_LIMIT_MUST_BE_GREATER_THAN_ZERO);
    }
}

bool Inventory::canAdd(const Stack& stack) const { return validStack(stack) && availableSpace(stack) >= stack.quantity; }

uint32_t Inventory::availableSpace(const Stack& stack) const {
    const uint16_t limit = getStackLimit(stack.item_id);
    uint32_t available = 0;

    for (uint16_t i = 0; i < capacity; ++i) {
        const auto it = slots.find(static_cast<uint8_t>(i));

        if (it == slots.end()) {
            available += limit;
        } else if (it->second.item_id == stack.item_id && it->second.qual == stack.qual) {
            available += limit - it->second.quantity;
        }
    }

    return available;
}

uint16_t Inventory::addUpTo(const Stack& stack) {
    if (!validStack(stack))
        return 0;
    const auto amount = static_cast<uint16_t>(std::min<uint32_t>(
        stack.quantity, availableSpace(stack)));
    if (amount != 0) {
        add(Stack{stack.item_id, amount, stack.qual});
    }
    return amount;
}

bool Inventory::add(const Stack& stack) {
    const Stack incoming = stack;

    if (!canAdd(incoming)) {
        return false;
    }

    const uint16_t limit = getStackLimit(incoming.item_id);
    uint16_t remaining = incoming.quantity;

    //completa stack si existe del mismo tipo y calidad
    for (auto& [slot, current] : slots) {
        if (current.item_id != incoming.item_id || current.qual != incoming.qual) {
            continue;
        }

        const uint16_t amount = std::min<uint16_t>(remaining, limit - current.quantity);
        current.quantity += amount;
        remaining -= amount;

        if (remaining == 0) {
            return true;
        }
    }

    //desp los slots vacios
    for (uint16_t i = 0; i < capacity && remaining > 0; ++i) {
        const auto slot = static_cast<uint8_t>(i);

        if (slots.contains(slot)) {
            continue;
        }

        const uint16_t amount = std::min(remaining, limit);
        slots.emplace(slot,Stack{incoming.item_id, amount, incoming.qual});
        remaining -= amount;
    }

    return true;
}

bool Inventory::remove(uint8_t slot, uint16_t quantity) {
    const auto it = slots.find(slot);

    if (quantity == 0 || it == slots.end() || it->second.quantity < quantity) {
        return false;
    }

    it->second.quantity -= quantity;

    if (it->second.quantity == 0) {
        slots.erase(it);
    }

    return true;
}

bool Inventory::clearSlot(uint8_t slot) {
    const auto it = slots.find(slot);
    if (it == slots.end() || isTool(it->second.item_id))
        return false;
    slots.erase(it);
    return true;
}

bool Inventory::transferTo(Inventory& destination, uint8_t slot, uint16_t quantity) {
    const auto* source = getSlot(slot);
    if (&destination == this || !source || quantity == 0 || source->quantity < quantity) {
        return false;
    }
    const Stack transferred{source->item_id, quantity, source->qual};
    Inventory prepared = destination;
    if (!prepared.add(transferred))
        return false;

    remove(slot, quantity);
    destination = std::move(prepared);
    return true;
}

const Stack* Inventory::getSlot(uint8_t slot) const {
    const auto it = slots.find(slot);
    return it == slots.end() ? nullptr : &it->second;
}

uint8_t Inventory::getCapacity() const {
    return capacity;
}

const std::map<uint8_t, Stack>& Inventory::getItems() const {
    return slots;
}

void Inventory::restoreSlots(const std::map<uint8_t, Stack>& saved) {
    for (const auto& [slot, stack] : saved) {
        if (slot >= capacity || !validStack(stack) || stack.quantity > getStackLimit(stack.item_id)) {
            throw std::invalid_argument(server_constants::INVALID_SAVED_INVENTORY_SLOT);
        }
    }
    slots = saved;
}
