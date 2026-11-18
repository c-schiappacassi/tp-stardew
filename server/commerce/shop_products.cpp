#include "shop_products.h"

#include <algorithm>
#include <cctype>

namespace {
struct ProductName {
    ItemId item;
    std::string_view name;
    std::string_view english;
};

constexpr ProductName names[]{
    {ItemId::StrawberrySeed, "semilla_frutilla", "strawberry_seed"},
    {ItemId::ScallionSeed, "semilla_cebolla_verdeo", "scallion_seed"},
    {ItemId::PotatoSeed, "semilla_papa", "potato_seed"},
    {ItemId::PumpkinSeed, "semilla_calabaza", "pumpkin_seed"},
    {ItemId::Strawberry, "frutilla", "strawberry"},
    {ItemId::Scallion, "cebolla_verdeo", "scallion"},
    {ItemId::Potato, "papa", "potato"}, {ItemId::Pumpkin, "calabaza", "pumpkin"},
    {ItemId::Coal, "carbon", "coal"}, {ItemId::Lapis, "lapislazuli", "lapis"},
    {ItemId::Silver, "plata", "silver"}, {ItemId::Gold, "oro", "gold"},
    {ItemId::Milk, "leche", "milk"}, {ItemId::Egg, "huevo", "egg"},
    {ItemId::Chest, "cofre", "chest"}, {ItemId::Chair, "silla", "chair"},
    {ItemId::Table, "mesa", "table"}, {ItemId::Bonsai, "bonsai", "bonsai"},
    {ItemId::PottedTree, "arbol_maceta", "potted_tree"},
    {ItemId::BlueArmCouch, "sillon_azul", "blue_arm_couch"},
    {ItemId::Tv, "televisor", "tv"}, {ItemId::BeigeRug, "alfombra_beige", "beige_rug"},
    {ItemId::ClockFirePlace, "chimenea_reloj", "clock_fire_place"},
    {ItemId::BlueRug, "alfombra_azul", "blue_rug"},
    {ItemId::CatFirePlace, "chimenea_gato", "cat_fire_place"},
    {ItemId::SimpleFirePlace, "chimenea", "simple_fire_place"},
    {ItemId::BlueJunimoPushee, "junimo_azul", "blue_junimo_pushee"},
    {ItemId::GreenJunimoPushee, "junimo_verde", "green_junimo_pushee"},
    {ItemId::YellowJunimoPushee, "junimo_amarillo", "yellow_junimo_pushee"},
    {ItemId::PinkJunimoPushee, "junimo_rosa", "pink_junimo_pushee"},
    {ItemId::SimpleLamp, "lampara", "simple_lamp"},
    {ItemId::BookShelf, "biblioteca", "book_shelf"},
    {ItemId::SquareLamp, "lampara_cuadrada", "square_lamp"},
    {ItemId::RoundTable, "mesa_redonda", "round_table"},
    {ItemId::GreenStool, "taburete_verde", "green_stool"},
    {ItemId::BrownSofa, "sofa_marron", "brown_sofa"},
    {ItemId::BrownArmCouch, "sillon_marron", "brown_arm_couch"}
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
    return "desconocido";
}

std::optional<ItemId> shop_products::item(const std::string& name) {
    const auto key = normalized(name);
    for (const auto& entry : names) {
        if (key == entry.name || key == entry.english) return entry.item;
    }
    if (key == "orbe_oro") return ItemId::Gold;
    if (key == "semilla_cebolla") return ItemId::ScallionSeed;
    return std::nullopt;
}

std::optional<AnimalSpecies> shop_products::animal(const std::string& name) {
    const auto key = normalized(name);
    if (key == "vaca" || key == "vacas" || key == "cow") return AnimalSpecies::Cow;
    if (key == "gallina" || key == "gallinas" || key == "chicken") return AnimalSpecies::Chicken;
    return std::nullopt;
}

std::string_view shop_products::merchantName(NpcId merchant) {
    switch (merchant) {
        case NpcId::Marnie: return "Marnie";
        case NpcId::Pierre: return "Pierre";
        case NpcId::Robin: return "Robin";
        case NpcId::Clint: return "Clint";
    }
    return "Comerciante";
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
    return static_cast<uint64_t>(basePrice) * quarters / 4;
}
