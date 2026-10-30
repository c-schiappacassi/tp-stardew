#pragma once

#include <cstdint>
#include <map>

#include "dtos.h"

class Inventory {
private:
    uint8_t capacity;
    uint16_t stackLimit;
    std::map<uint8_t, Stack> slots;
    bool canAdd(const Stack& stack) const;
    uint16_t getStackLimit(ItemId item) const;

public:
    explicit Inventory(uint8_t capacity, uint16_t stackLimit = 50);
    bool add(const Stack& stack);
    bool remove(uint8_t slot, uint16_t quantity);
    uint8_t getCapacity() const;
    const Stack* getSlot(uint8_t slot) const;
    const std::map<uint8_t, Stack>& getItems() const;
};