#include "shop_products.h"

#include "server_constants.h"

#include <algorithm>
#include <cctype>
#include <iterator>

#include "farm/farm_product.h"

namespace {
struct ProductName {
    ItemId item;
    std::string_view name;
};

constexpr ProductName names[]{
    {ItemId::StrawberrySeed, server_constants::STRAWBERRY_SEED},
    {ItemId::ScallionSeed, server_constants::SCALLION_SEED},
    {ItemId::PotatoSeed, server_constants::POTATO_SEED},
    {ItemId::PumpkinSeed, server_constants::PUMPKIN_SEED},
    {ItemId::Strawberry, server_constants::STRAWBERRY},
    {ItemId::Scallion, server_constants::SCALLION},
    {ItemId::Potato, server_constants::POTATO}, {ItemId::Pumpkin, server_constants::PUMPKIN},
    {ItemId::Coal, server_constants::COAL_2}, {ItemId::Lapis, server_constants::LAPIS_2},
    {ItemId::Silver, server_constants::SILVER_2}, {ItemId::Gold, server_constants::GOLD_2},
    {ItemId::Milk, server_constants::MILK}, {ItemId::Egg, server_constants::EGG},
    {ItemId::Chest, server_constants::CHEST}, {ItemId::Chair, server_constants::CHAIR},
    {ItemId::Table, server_constants::TABLE}, {ItemId::Bonsai, server_constants::BONSAI},
    {ItemId::PottedTree, server_constants::POTTED_TREE},
    {ItemId::BlueArmCouch, server_constants::BLUE_ARM_COUCH},
    {ItemId::Tv, server_constants::TV}, {ItemId::BeigeRug, server_constants::BEIGE_RUG},
    {ItemId::ClockFirePlace, server_constants::CLOCK_FIRE_PLACE},
    {ItemId::BlueRug, server_constants::BLUE_RUG},
    {ItemId::CatFirePlace, server_constants::CAT_FIRE_PLACE},
    {ItemId::SimpleFirePlace, server_constants::SIMPLE_FIRE_PLACE},
    {ItemId::BlueJunimoPushee, server_constants::BLUE_JUNIMO_PUSHEE},
    {ItemId::GreenJunimoPushee, server_constants::GREEN_JUNIMO_PUSHEE},
    {ItemId::YellowJunimoPushee, server_constants::YELLOW_JUNIMO_PUSHEE},
    {ItemId::PinkJunimoPushee, server_constants::PINK_JUNIMO_PUSHEE},
    {ItemId::SimpleLamp, server_constants::SIMPLE_LAMP},
    {ItemId::BookShelf, server_constants::BOOK_SHELF},
    {ItemId::SquareLamp, server_constants::SQUARE_LAMP},
    {ItemId::RoundTable, server_constants::ROUND_TABLE},
    {ItemId::GreenStool, server_constants::GREEN_STOOL},
    {ItemId::BrownSofa, server_constants::BROWN_SOFA},
    {ItemId::BrownArmCouch, server_constants::BROWN_ARM_COUCH}
};

std::string normalized(std::string name) {
    std::transform(name.begin(), name.end(), name.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return name;
}
}

std::string_view shop_products::name(ItemId item) {
    for (const auto& entry : names) {
        if (entry.item == item) return entry.name;
    }
    return server_constants::UNKNOWN;
}

std::optional<ItemId> shop_products::item(const std::string& name) {
    const auto key = normalized(name);
    for (const auto& entry : names) {
        if (key == entry.name) return entry.item;
    }
    return std::nullopt;
}

std::vector<std::string_view> shop_products::itemNames() {
    std::vector<std::string_view> result;
    result.reserve(std::size(names));
    for (const auto& entry: names)
        result.push_back(entry.name);
    return result;
}

std::optional<AnimalSpecies> shop_products::animal(const std::string& name) {
    const auto key = normalized(name);
    if (key == server_constants::COW) return AnimalSpecies::Cow;
    if (key == server_constants::CHICKEN) return AnimalSpecies::Chicken;
    return std::nullopt;
}

std::optional<AnimalType> shop_products::animalVariant(const std::string& name) {
    const auto key = normalized(name);
    if (key == server_constants::BROWN_COW) return AnimalType::BrownCow;
    if (key == server_constants::WHITE_COW) return AnimalType::WhiteCow;
    if (key == server_constants::BROWN_CHICKEN) return AnimalType::BrownChicken;
    if (key == server_constants::WHITE_CHICKEN) return AnimalType::WhiteChicken;
    if (key == server_constants::BLUE_CHICKEN) return AnimalType::BlueChicken;
    if (key == server_constants::VOID_CHICKEN) return AnimalType::VoidChicken;
    return std::nullopt;
}

std::string_view shop_products::merchantName(NpcId merchant) {
    switch (merchant) {
        case NpcId::Marnie: return server_constants::MARNIE;
        case NpcId::Pierre: return server_constants::PIERRE;
        case NpcId::Robin: return server_constants::ROBIN;
        case NpcId::Clint: return server_constants::CLINT;
    }
    return server_constants::MERCHANT;
}

std::optional<NpcId> shop_products::merchantForInterior(InteriorId interior) {
    switch (interior) {
        case InteriorId::MarniesShop: return NpcId::Marnie;
        case InteriorId::PierresShop: return NpcId::Pierre;
        case InteriorId::RobinsShop: return NpcId::Robin;
        case InteriorId::ClintsShop: return NpcId::Clint;
        default: return std::nullopt;
    }
}

std::optional<uint64_t> shop_products::salePrice(uint32_t basePrice, Quality quality) {
    const auto quarters = salePriceQuarters(basePrice, quality);
    return quarters ? std::optional<uint64_t>{*quarters / 4} : std::nullopt;
}

std::optional<uint64_t> shop_products::salePriceQuarters(uint32_t basePrice, Quality quality) {
    uint64_t quarters;
    switch (quality) {
        case Quality::None:
        case Quality::Base: quarters = 4; break;
        case Quality::Good: quarters = 5; break;
        case Quality::Excellent: quarters = 8; break;
        case Quality::Incredible: quarters = 16; break;
        case Quality::Perfect: quarters = 32; break;
        default: return std::nullopt;
    }
    return static_cast<uint64_t>(basePrice) * quarters;
}

std::optional<ItemId> shop_products::cropItem(CropType type) { return farm_products::cropItem(type); }
