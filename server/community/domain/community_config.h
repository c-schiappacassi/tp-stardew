#pragma once

#include <cstdint>
#include <map>

#include "protocol_types.h"

struct CommunityConfig {
    std::map<ItemId, uint32_t> requirements{
        {ItemId::Strawberry, 3}, {ItemId::Scallion, 3},
        {ItemId::Potato, 3}, {ItemId::Pumpkin, 3},
        {ItemId::Coal, 10}, {ItemId::Lapis, 7}, {ItemId::Silver, 5}, {ItemId::Gold, 3},
        {ItemId::Milk, 5}, {ItemId::Egg, 10}
    };
    uint32_t moneyRequired = 10000;
    uint16_t seedReward = 5;
    uint32_t metalGoldReward = 500;

    void validate() const;
};
