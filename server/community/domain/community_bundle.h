#pragma once

#include <array>
#include <optional>
#include <string_view>

#include "protocol_types.h"

namespace community_bundle {
inline constexpr std::array all{CommunityBundle::Crops, CommunityBundle::Metals,CommunityBundle::Animals, CommunityBundle::Combat, CommunityBundle::Money};

inline constexpr std::array requiredItems{ItemId::Strawberry, ItemId::Scallion,ItemId::Potato, ItemId::Pumpkin, ItemId::Coal, ItemId::Lapis,ItemId::Silver, ItemId::Gold, ItemId::Milk, ItemId::Egg};

inline constexpr std::array cropRewardSeeds{ItemId::StrawberrySeed, ItemId::ScallionSeed,ItemId::PotatoSeed, ItemId::PumpkinSeed};

std::string_view name(CommunityBundle bundle);
std::optional<CommunityBundle> fromName(std::string_view name);
std::optional<CommunityBundle> forItem(ItemId item);
bool acceptsQuality(ItemId item, Quality quality);
}
