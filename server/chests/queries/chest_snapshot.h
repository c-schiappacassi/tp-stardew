#pragma once

#include "dtos.h"

class Chest;

class ChestSnapshot {
private:
    FurnitureData data;
    OpenChestData contents;

public:
    explicit ChestSnapshot(const Chest& chest);

    uint32_t getId() const;
    const Position& getPosition() const;
    bool isEmpty() const;
    const FurnitureData& getPublicData() const;
    const OpenChestData& getContents() const;
};
