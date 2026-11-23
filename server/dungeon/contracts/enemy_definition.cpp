#include "enemy_definition.h"

#include <initializer_list>
#include <stdexcept>

void EnemyConfig::validate() const {
    for (auto type: {EnemyType::Skeleton, EnemyType::Orc, EnemyType::VampireMale, EnemyType::VampireFemale, EnemyType::Skull}) {
        const auto it = definitions.find(type);
        if (it == definitions.end() || it->second.health == 0 || it->second.damage == 0 || it->second.damage > 32767 || it->second.attackMs == 0 || it->second.range == 0 ||
            (type != EnemyType::Skull && it->second.speed == 0)) {
            throw std::invalid_argument("Invalid enemy definition");
        }
    }
    if (skeletonReviveMs == 0 || vampireReviveMs == 0 || flameSpeed < 2 || flameLifetimeMs == 0 || trapInactiveMs == 0 || trapWarningMs == 0 || trapActiveMs == 0 ||
        trapHitMs == 0 || trapDamage == 0 || orcSilverChance > 100 || miningAmbushChance > 100 || goldChance > 100 || goldMin > goldMax || maxEnemiesPerRoom < 6 ||
        maxEnemiesPerRoom > 256 || skeletonsPerRoom > maxEnemiesPerRoom || static_cast<unsigned>(spikesPerRoom) + wallFlamesPerRoom > 256) {
        throw std::invalid_argument("Invalid combat configuration");
    }
}
