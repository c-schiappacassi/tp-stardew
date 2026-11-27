#include "farm_config.h"

#include "server_constants.h"

#include <limits>
#include <set>
#include <stdexcept>

#include "farm_product.h"

void FarmConfig::validateDayLength(std::chrono::milliseconds dayLength) {
    if (dayLength.count() < 6 || dayLength.count() > std::numeric_limits<int64_t>::max() / 7) {
        throw std::invalid_argument(server_constants::DAY_DURATION_DOES_NOT_ALLOW_DEFINING_FARM_CYCLES);
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
            throw std::invalid_argument(server_constants::UNKNOWN_CROP_TYPE);
        }

        const bool invalidTolerance = crop.toleratedMissedWaterings != 0 &&
            crop.wateringInterval.count() >
                std::numeric_limits<int64_t>::max() / crop.toleratedMissedWaterings;
        if (!cropTypes.insert(crop.type).second || crop.growthTime.count() <= 0 ||
            crop.wateringInterval.count() <= 0 || crop.regrowthTime.count() < 0 ||
            invalidTolerance) {
            throw std::invalid_argument(server_constants::DUPLICATE_CROP_DEFINITION_OR_ZERO_DURATION);
        }
    }

    std::set<AnimalSpecies> animalTypes;

    for (const auto& animal : animals) {
        animal.movement.validate();
        if (animal.species != AnimalSpecies::Cow && animal.species != AnimalSpecies::Chicken) {
            throw std::invalid_argument(server_constants::UNKNOWN_ANIMAL_TYPE);
        }

        if (!animalTypes.insert(animal.species).second || animal.productionInterval.count() <= 0 ||
            animal.maxCount == 0 || !animal.collision.isValid()) {
            throw std::invalid_argument(server_constants::DUPLICATE_ANIMAL_DEFINITION_OR_ZERO_DURATION_OR_LIMIT);
        }
    }
}

std::optional<uint32_t> FarmConfig::animalBasePrice(AnimalSpecies species) const {
    for (const auto& animal: animals) {
        if (animal.species == species)
            return animal.basePrice;
    }
    return std::nullopt;
}

std::optional<uint32_t> FarmConfig::productBasePrice(ItemId item) const {
    std::optional<uint32_t> price;
    for (const auto& crop: crops) {
        if (farm_products::cropItem(crop.type) == item)
            price = crop.basePrice;
    }
    for (const auto& animal: animals) {
        const auto product = animal.species == AnimalSpecies::Cow ? ItemId::Milk : ItemId::Egg;
        if (product == item)
            price = animal.productPrice;
    }
    return price;
}

FarmPrices FarmConfig::prices() const {
    FarmPrices result;
    for (const auto& crop: crops) {
        if (const auto item = farm_products::cropItem(crop.type))
            result.products.push_back({*item, crop.basePrice});
    }
    for (const auto& animal: animals) {
        result.products.push_back({animal.species == AnimalSpecies::Cow ? ItemId::Milk : ItemId::Egg, animal.productPrice});
        result.animals.push_back({animal.species, animal.basePrice});
    }
    return result;
}
