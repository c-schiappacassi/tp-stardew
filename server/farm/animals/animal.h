#pragma once

#include <cstdint>
#include <chrono>
#include <optional>
#include <random>
#include "dtos.h"
#include "animal_movement.h"
#include "animal_species.h"
#include "entities/entity_id.h"
#include "farm/farm_product.h"

class AnimalManager;

class Animal {
private:
    EntityId id;
    AnimalType type;
    AnimalMovement movement;
    std::chrono::milliseconds productionInterval;
    std::chrono::milliseconds productionElapsed{0};
    uint32_t availableProducts = 0;

    friend class AnimalManager;
    bool moveTo(const Position& base, FarmPlacement& placement);
    void advanceMovement(FarmPlacement& placement, std::mt19937& random);

protected:
    enum class ProductionMode {
        WaitForCollection,
        Accumulate
    };

    Animal(EntityId id, const Position& base, AnimalType type,std::chrono::milliseconds productionInterval,const AnimalMovementConfig& movementConfig = {});
    void advanceProduction(std::chrono::milliseconds elapsed, ProductionMode mode);

public:
    virtual ~Animal();
    EntityId getId() const;
    const Position& getPosition() const;
    Direction getDirection() const;
    AnimalType getType() const;
    uint32_t getAvailableProducts() const;
    void restoreProgress(std::chrono::milliseconds elapsed, uint32_t products);
    std::chrono::milliseconds getProductionElapsed() const;
    virtual bool needsInteraction() const;
    virtual bool dropsProductsAutomatically() const;
    virtual AnimalSpecies getSpecies() const = 0;
    virtual ItemId getProduct() const = 0;
    virtual AnimalState getState() const;
    virtual void updateProduction(std::chrono::milliseconds elapsed) = 0;
    std::optional<FarmProduct> getProduction() const;
    AnimalData getPublicData() const;
    std::optional<FarmProduct> collect();
};
