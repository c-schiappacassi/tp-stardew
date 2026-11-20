#pragma once

#include <cstdint>
#include <map>
#include <stdexcept>

#include "protocol_types.h"

struct EnemyDefinition {
    uint16_t health = 40;
    uint16_t damage = 8;
    uint16_t speed = 32;
    uint16_t range = 20;
    uint32_t attackMs = 1000;
};

struct EnemyConfig {
    std::map<EnemyType, EnemyDefinition> definitions{
            {EnemyType::Skeleton, {40, 8, 32, 20, 1000}},
            {EnemyType::Orc, {70, 12, 36, 20, 1200}},
            {EnemyType::VampireMale, {100, 18, 40, 22, 1000}},
            {EnemyType::VampireFemale, {90, 12, 60, 22, 900}},
            {EnemyType::Skull, {1, 10, 0, 160, 1500}}};
    uint32_t skeletonReviveMs = 7000;
    uint32_t vampireReviveMs = 3000;
    uint16_t bloodRadius = 80;
    uint16_t flameSpeed = 80;
    uint32_t flameLifetimeMs = 4000;
    uint32_t trapInactiveMs = 2000;
    uint32_t trapWarningMs = 700;
    uint32_t trapActiveMs = 1000;
    uint32_t trapHitMs = 800;
    uint16_t trapDamage = 12;
    uint16_t wallFlameRange = 64;
    uint16_t spawnSafetyRange = 64;
    uint8_t skeletonsPerRoom = 1;
    uint8_t orcGroupEvery = 3;
    uint8_t vampirePairEvery = 4;
    uint8_t skullEvery = 5;
    uint8_t orcSilverChance = 20;
    uint8_t miningAmbushChance = 10;
    uint8_t goldChance = 60;
    uint16_t goldMin = 2;
    uint16_t goldMax = 10;
    uint16_t maxEnemiesPerRoom = 32;

    void validate() const {
        for (auto type: {EnemyType::Skeleton, EnemyType::Orc, EnemyType::VampireMale,
                         EnemyType::VampireFemale, EnemyType::Skull}) {
            const auto it = definitions.find(type);
            if (it == definitions.end() || it->second.health == 0 || it->second.damage == 0 ||
                it->second.damage > 32767 || it->second.attackMs == 0 || it->second.range == 0 ||
                (type != EnemyType::Skull && it->second.speed == 0)) {
                throw std::invalid_argument("Invalid enemy definition");
            }
        }
        if (skeletonReviveMs == 0 || vampireReviveMs == 0 || flameSpeed < 2 ||
            flameLifetimeMs == 0 || trapInactiveMs == 0 || trapWarningMs == 0 ||
            trapActiveMs == 0 || trapHitMs == 0 || trapDamage == 0 || orcSilverChance > 100 ||
            miningAmbushChance > 100 || goldChance > 100 || goldMin > goldMax ||
            maxEnemiesPerRoom < 6 || maxEnemiesPerRoom > 256 ||
            skeletonsPerRoom > maxEnemiesPerRoom) {
            throw std::invalid_argument("Invalid combat configuration");
        }
    }
};
