#pragma once

#include <cstdint>
#include <map>
#include "protocol_types.h"

struct ToolConfig {
    uint16_t energyCost;
    uint32_t intervalMs;
};

struct PlayerConfig {
    std::map<ItemId, ToolConfig> tools{
        {ItemId::Pickaxe,{2,500}},
        {ItemId::WateringCan, {2,500}},
        {ItemId::Shovel,{2,500}},
        {ItemId::Sword,{2,500}}
    };
};