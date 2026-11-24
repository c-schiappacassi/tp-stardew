#pragma once

#include <cstdint>
#include <string>
#include <variant>

struct CommunityStatusCommand {
    std::string bundle;
};

struct CommunityRewardsCommand {};

struct DonateItemCommand {
    std::string product;
    uint32_t quantity;
};

struct DonateCoinsCommand {
    uint32_t quantity;
};

using CommunityCommand = std::variant<CommunityStatusCommand, CommunityRewardsCommand, DonateItemCommand, DonateCoinsCommand>;
