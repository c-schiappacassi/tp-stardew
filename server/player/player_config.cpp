#include "player_config.h"

#include <stdexcept>

void PlayerConfig::validate() const {
    const auto sword = tools.find(ItemId::Sword);
    if (!collision.isValid() || !hitbox.isValid() || swordDamage == 0 || swordRange == 0 ||
        passoutGoldPercent > 100 || dayDurationMs == 0 || sword == tools.end() ||
        sword->second.intervalMs == 0) {
        throw std::invalid_argument("Invalid player configuration");
    }
}
