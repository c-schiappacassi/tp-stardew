#pragma once

#include <cstdint>
#include <map>
#include <stdexcept>

#include "entities/collision_box.h"

#include "protocol_types.h"

struct ToolConfig {
    uint16_t energyCost;
    uint32_t intervalMs;
};

struct PlayerConfig {
    uint16_t swordDamage = 20;
    uint16_t swordRange = 40;
    uint32_t damageGraceMs = 350;
    uint8_t passoutGoldPercent = 10;
    uint32_t dayDurationMs = 900000;
    std::map<ItemId, ToolConfig> tools{
                                        {ItemId::Pickaxe,{2,500}},
        {ItemId::WateringCan, {2,500}},
        {ItemId::Shovel,{2,500}},
        {ItemId::Sword,{2,500}}
    };
    CollisionBox collision{entity_dimensions::PLAYER};

    void validate() const {
        const auto sword = tools.find(ItemId::Sword);
        if (!collision.isValid() || swordDamage == 0 || swordRange == 0 ||
            passoutGoldPercent > 100 || dayDurationMs == 0 || sword == tools.end() ||
            sword->second.intervalMs == 0) {
            throw std::invalid_argument("Configuracion del jugador invalida");
        }
    }
};
