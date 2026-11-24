#include "community_bundle.h"

bool community_bundle::acceptsQuality(ItemId item, Quality quality) {
    const auto bundle = forItem(item);
    if (!bundle) return false;
    return *bundle != CommunityBundle::Crops ||
        quality == Quality::Incredible || quality == Quality::Perfect;
}

std::string_view community_bundle::name(CommunityBundle bundle) {
    switch (bundle) {
        case CommunityBundle::Crops: return "crops";
        case CommunityBundle::Metals: return "metals";
        case CommunityBundle::Animals: return "animals";
        case CommunityBundle::Combat: return "combat";
        case CommunityBundle::Money: return "money";
    }
    return "unknown";
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
