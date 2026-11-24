#pragma once

#include "animal.h"

class Chicken final : public Animal {
public:
    Chicken(EntityId id, const Position& position, AnimalType type,
        std::chrono::milliseconds productionInterval,
        const AnimalMovementConfig& movementConfig = {});

    AnimalSpecies getSpecies() const override;
    ItemId getProduct() const override;
    bool dropsProductsAutomatically() const override;
    void updateProduction(std::chrono::milliseconds elapsed) override;
};
