#pragma once

#include <cstdint>
#include <map>

#include "constants.h"
#include "dtos.h"

class Inventory {
private:
    uint8_t capacity;
    uint16_t stackLimit;
    std::map<uint8_t, Stack> slots;
    bool canAdd(const Stack& stack) const;
    uint32_t availableSpace(const Stack& stack) const;
    uint16_t getStackLimit(ItemId item) const;
    static bool isTool(ItemId item);

public:
    explicit Inventory(uint8_t capacity, uint16_t stackLimit = INVENTORY_STACK_LIMIT);
    bool add(const Stack& stack);
    uint16_t addUpTo(const Stack& stack);
    bool remove(uint8_t slot, uint16_t quantity);
    bool clearSlot(uint8_t slot);
    bool transferTo(Inventory& destination, uint8_t slot, uint16_t quantity);
    uint8_t getCapacity() const;
    const Stack* getSlot(uint8_t slot) const;
    const std::map<uint8_t, Stack>& getItems() const;
    void restoreSlots(const std::map<uint8_t, Stack>& saved);
};
