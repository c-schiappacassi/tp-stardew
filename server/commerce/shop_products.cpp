#include "shop_products.h"

#include <algorithm>
#include <cctype>

namespace {
struct ProductName {
    ItemId item;
    std::string_view name;
};

constexpr ProductName names[]{
    {ItemId::StrawberrySeed, "strawberry_seed"},
    {ItemId::ScallionSeed, "scallion_seed"},
    {ItemId::PotatoSeed, "potato_seed"},
    {ItemId::PumpkinSeed, "pumpkin_seed"},
    {ItemId::Strawberry, "strawberry"},
    {ItemId::Scallion, "scallion"},
    {ItemId::Potato, "potato"}, {ItemId::Pumpkin, "pumpkin"},
    {ItemId::Coal, "coal"}, {ItemId::Lapis, "lapis"},
    {ItemId::Silver, "silver"}, {ItemId::Gold, "gold"},
    {ItemId::Milk, "milk"}, {ItemId::Egg, "egg"},
    {ItemId::Chest, "chest"}, {ItemId::Chair, "chair"},
    {ItemId::Table, "table"}, {ItemId::Bonsai, "bonsai"},
    {ItemId::PottedTree, "potted_tree"},
    {ItemId::BlueArmCouch, "blue_arm_couch"},
    {ItemId::Tv, "tv"}, {ItemId::BeigeRug, "beige_rug"},
    {ItemId::ClockFirePlace, "clock_fire_place"},
    {ItemId::BlueRug, "blue_rug"},
    {ItemId::CatFirePlace, "cat_fire_place"},
    {ItemId::SimpleFirePlace, "simple_fire_place"},
    {ItemId::BlueJunimoPushee, "blue_junimo_pushee"},
    {ItemId::GreenJunimoPushee, "green_junimo_pushee"},
    {ItemId::YellowJunimoPushee, "yellow_junimo_pushee"},
    {ItemId::PinkJunimoPushee, "pink_junimo_pushee"},
    {ItemId::SimpleLamp, "simple_lamp"},
    {ItemId::BookShelf, "book_shelf"},
    {ItemId::SquareLamp, "square_lamp"},
    {ItemId::RoundTable, "round_table"},
    {ItemId::GreenStool, "green_stool"},
    {ItemId::BrownSofa, "brown_sofa"},
    {ItemId::BrownArmCouch, "brown_arm_couch"}
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
    return "unknown";
}

std::optional<ItemId> shop_products::item(const std::string& name) {
    const auto key = normalized(name);
    for (const auto& entry : names) {
        if (key == entry.name) return entry.item;
    }
    return std::nullopt;
}

std::optional<AnimalSpecies> shop_products::animal(const std::string& name) {
    const auto key = normalized(name);
    if (key == "cow") return AnimalSpecies::Cow;
    if (key == "chicken") return AnimalSpecies::Chicken;
    return std::nullopt;
}

std::optional<AnimalType> shop_products::animalVariant(const std::string& name) {
    const auto key = normalized(name);
    if (key == "brown_cow") return AnimalType::BrownCow;
    if (key == "white_cow") return AnimalType::WhiteCow;
    if (key == "brown_chicken") return AnimalType::BrownChicken;
    if (key == "white_chicken") return AnimalType::WhiteChicken;
    if (key == "blue_chicken") return AnimalType::BlueChicken;
    if (key == "void_chicken") return AnimalType::VoidChicken;
    return std::nullopt;
}

std::string_view shop_products::merchantName(NpcId merchant) {
    switch (merchant) {
        case NpcId::Marnie: return "Marnie";
        case NpcId::Pierre: return "Pierre";
        case NpcId::Robin: return "Robin";
        case NpcId::Clint: return "Clint";
    }
    return "Merchant";
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

std::optional<ItemId> shop_products::cropItem(CropType type) {
    switch (type) {
        case CropType::Strawberry: return ItemId::Strawberry;
        case CropType::Scallion: return ItemId::Scallion;
        case CropType::Potato: return ItemId::Potato;
        case CropType::Pumpkin: return ItemId::Pumpkin;
    }
    return std::nullopt;
}
