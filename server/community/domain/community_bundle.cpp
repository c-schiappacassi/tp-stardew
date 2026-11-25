#include "community_bundle.h"

#include "server_constants.h"

bool community_bundle::acceptsQuality(ItemId item, Quality quality) {
    const auto bundle = forItem(item);
    if (!bundle) return false;
    return *bundle != CommunityBundle::Crops ||
        quality == Quality::Incredible || quality == Quality::Perfect;
}

std::string_view community_bundle::name(CommunityBundle bundle) {
    switch (bundle) {
        case CommunityBundle::Crops: return server_constants::CROPS;
        case CommunityBundle::Metals: return server_constants::METALS;
        case CommunityBundle::Animals: return server_constants::ANIMALS;
        case CommunityBundle::Combat: return server_constants::COMBAT;
        case CommunityBundle::Money: return server_constants::MONEY;
    }
    return server_constants::UNKNOWN;
}

std::optional<CommunityBundle> community_bundle::fromName(std::string_view text) {
    for (const auto bundle : all) {
        if (name(bundle) == text) return bundle;
    }
    return std::nullopt;
}

std::optional<CommunityBundle> community_bundle::forItem(ItemId item) {
    switch (item) {
        case ItemId::Strawberry:
        case ItemId::Scallion:
        case ItemId::Potato:
        case ItemId::Pumpkin: return CommunityBundle::Crops;
        case ItemId::Coal:
        case ItemId::Lapis:
        case ItemId::Silver:
        case ItemId::Gold: return CommunityBundle::Metals;
        case ItemId::Milk:
        case ItemId::Egg: return CommunityBundle::Animals;
        default: return std::nullopt;
    }
}
