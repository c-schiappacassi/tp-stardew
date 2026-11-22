#include "animal.h"

#include <limits>
#include <algorithm>
#include <stdexcept>

Animal::Animal(EntityId id, const Position& position, AnimalType type,std::chrono::milliseconds productionInterval, const AnimalMovementConfig& movementConfig):
    id(id), type(type), movement(position, movementConfig), productionInterval(productionInterval) {
    if (id == 0 || productionInterval.count() <= 0) {
        throw std::invalid_argument("Invalid animal identifier or time");
    }
}

Animal::~Animal() = default;

EntityId Animal::getId() const { return id; }

const Position& Animal::getPosition() const { return movement.getPosition(); }

Direction Animal::getDirection() const { return movement.getDirection(); }

AnimalType Animal::getType() const { return type; }

uint32_t Animal::getAvailableProducts() const { return availableProducts; }
std::chrono::milliseconds Animal::getProductionElapsed() const { return productionElapsed; }

bool Animal::needsInteraction() const { return false; }

bool Animal::dropsProductsAutomatically() const { return false; }

AnimalState Animal::getState() const {
    if (movement.isWalking()) {
        return AnimalState::Walking;
    } else {
        return AnimalState::Idle;
    }
}

bool Animal::moveTo(const Position& position, FarmPlacement& placement) {
    return movement.moveTo(id, position, placement);
}

void Animal::advanceMovement(FarmPlacement& placement, std::mt19937& random) {
    movement.tick(id, placement, random);
}

void Animal::advanceProduction(std::chrono::milliseconds elapsed, ProductionMode mode) {
    if (elapsed.count() < 0) {
        throw std::invalid_argument("Negative time");
    }
    uint64_t produced = static_cast<uint64_t>(elapsed / productionInterval);
    const auto remainder = elapsed % productionInterval;
    const auto remaining = productionInterval - productionElapsed;
    if (remainder >= remaining) {
        ++produced;
        productionElapsed = remainder - remaining;
    } else {
        productionElapsed += remainder;
    }
    if (produced == 0) {
        return;
    }
    if (mode == ProductionMode::WaitForCollection) {
        availableProducts = 1;
        productionElapsed = std::chrono::milliseconds{0};
        return;
    }
    const uint64_t capacity = std::numeric_limits<uint32_t>::max() - availableProducts;
    availableProducts += static_cast<uint32_t>(std::min(produced, capacity));
}

std::optional<FarmProduct> Animal::getProduction() const {
    if (availableProducts == 0) {
        return std::nullopt;
    }
    return FarmProduct{getProduct(), availableProducts, Quality::Base};
}

AnimalData Animal::getPublicData() const {
    return AnimalData{id, getPosition(), getDirection(), type, getState(), needsInteraction()};
}

std::optional<FarmProduct> Animal::collect() {
    auto result = getProduction();
    availableProducts = 0;
    return result;
}

void Animal::restoreProgress(std::chrono::milliseconds elapsed, uint32_t products) {
    if (elapsed.count() < 0 || elapsed >= productionInterval) {
        throw std::invalid_argument("Invalid saved animal progress");
    }
    productionElapsed = elapsed;
    availableProducts = products;
}
