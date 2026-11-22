#pragma once

#include <cstdint>
#include <map>
#include <stdexcept>

#include "entities/collision_box.h"

#include "protocol_types.h"
#include "game_time.h"

struct ToolConfig {
    uint16_t energyCost;
    uint32_t intervalMs;
};

struct PlayerConfig {
    uint16_t swordDamage = 20;
    uint16_t swordRange = 40;
    uint32_t damageGraceMs = 350;
    uint8_t passoutGoldPercent = 10;
    uint32_t dayDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(game_time::DAY_DURATION).count();
    std::map<ItemId, ToolConfig> tools{
                                        {ItemId::Pickaxe,{2,500}},
        {ItemId::WateringCan, {2,500}},
        {ItemId::Shovel,{2,500}},
        {ItemId::Sword,{2,500}}
    };
    CollisionBox collision{entity_dimensions::PLAYER};
    CollisionBox hitbox{entity_dimensions::PLAYER_HITBOX};

    void validate() const {
        const auto sword = tools.find(ItemId::Sword);
        if (!collision.isValid() || !hitbox.isValid() || swordDamage == 0 || swordRange == 0 ||
            passoutGoldPercent > 100 || dayDurationMs == 0 || sword == tools.end() ||
            sword->second.intervalMs == 0) {
            throw std::invalid_argument("Invalid player configuration");
        }
    }
};
