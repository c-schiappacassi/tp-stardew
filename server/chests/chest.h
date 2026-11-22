#pragma once

#include "entities/entity_id.h"
#include "inventory/inventory.h"
#include "queries/chest_snapshot.h"

class Chest {
    friend class ChestCommands;
    friend class ChestSnapshot;

private:
    EntityId id;
    Position position;
    Inventory inventory{CHEST_INVENTORY_CAPACITY};

public:
    Chest(EntityId id, const Position& position, const std::map<uint8_t, Stack>& items = {});

    EntityId getId() const;
    const Position& getPosition() const;
    bool isEmpty() const;
    const Inventory& getInventory() const;
    FurnitureData getPublicData() const;
    ChestSnapshot snapshot() const;
};
