#pragma once

#include "animal.h"

class Cow final : public Animal {
public:
    Cow(EntityId id, const Position& position, AnimalType type,
        std::chrono::milliseconds productionInterval,
        const AnimalMovementConfig& movementConfig = {});

    AnimalSpecies getSpecies() const override;
    ItemId getProduct() const override;
    bool needsInteraction() const override;
    void updateProduction(std::chrono::milliseconds elapsed) override;
};
