#include "chest_snapshot.h"

#include "chests/chest.h"

ChestSnapshot::ChestSnapshot(const Chest& chest):
    data{chest.id, chest.position, FurnitureId::Chest, Direction::Down}, contents{chest.id, chest.inventory.getCapacity(), chest.inventory.getItems()} {}

uint32_t ChestSnapshot::getId() const { return data.id; }
const Position& ChestSnapshot::getPosition() const { return data.pos; }
bool ChestSnapshot::isEmpty() const { return contents.items.empty(); }
const FurnitureData& ChestSnapshot::getPublicData() const { return data; }
const OpenChestData& ChestSnapshot::getContents() const { return contents; }
