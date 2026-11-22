#include "farm_config.h"

#include <limits>
#include <set>
#include <stdexcept>

namespace {
void validateDayLength(std::chrono::milliseconds dayLength) {
    if (dayLength.count() < 6 || dayLength.count() > std::numeric_limits<int64_t>::max() / 7) {
        throw std::invalid_argument("Day duration does not allow defining farm cycles");
    }
}
}

FarmConfig::FarmConfig(std::chrono::milliseconds dayLength): dayLength(dayLength) {
    validateDayLength(dayLength);

    crops = {
        {CropType::Strawberry, 4 * dayLength, dayLength / 2, dayLength, 0, 25},
        {CropType::Scallion, 2 * dayLength, dayLength / 3, std::chrono::milliseconds{0}, 0, 12},
        {CropType::Potato, 3 * dayLength, dayLength / 2, std::chrono::milliseconds{0}, 1, 18},
        {CropType::Pumpkin, 7 * dayLength, dayLength, std::chrono::milliseconds{0}, 0, 50}
    };

    animals = {
        {AnimalSpecies::Cow, 2 * dayLength, 5, 80, 30, {},
            CollisionBox{entity_dimensions::COW}},
        {AnimalSpecies::Chicken, dayLength, 10, 20, 3, {},
            CollisionBox{entity_dimensions::CHICKEN}}
    };
}

void FarmConfig::validate() const {
    validateDayLength(dayLength);

    std::set<CropType> cropTypes;

    for (const auto& crop : crops) {
        if (crop.type != CropType::Strawberry && crop.type != CropType::Scallion &&
            crop.type != CropType::Potato && crop.type != CropType::Pumpkin) {
            throw std::invalid_argument("Unknown crop type");
        }

        const bool invalidTolerance = crop.toleratedMissedWaterings != 0 &&
            crop.wateringInterval.count() >
                std::numeric_limits<int64_t>::max() / crop.toleratedMissedWaterings;
        if (!cropTypes.insert(crop.type).second || crop.growthTime.count() <= 0 ||
            crop.wateringInterval.count() <= 0 || crop.regrowthTime.count() < 0 ||
            invalidTolerance) {
            throw std::invalid_argument("Duplicate crop definition or zero duration");
        }
    }

    std::set<AnimalSpecies> animalTypes;

    for (const auto& animal : animals) {
        animal.movement.validate();
        if (animal.species != AnimalSpecies::Cow && animal.species != AnimalSpecies::Chicken) {
            throw std::invalid_argument("Unknown animal type");
        }

        if (!animalTypes.insert(animal.species).second || animal.productionInterval.count() <= 0 ||
            animal.maxCount == 0 || !animal.collision.isValid()) {
            throw std::invalid_argument("Duplicate animal definition or zero duration or limit");
        }
    }
}
