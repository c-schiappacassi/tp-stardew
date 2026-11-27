#include "player_vitals.h"

#include "server_constants.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

#include "dtos.h"

uint16_t PlayerVitals::improvedMaximum(uint16_t value) {
    const uint32_t improved = static_cast<uint32_t>(value) * 5 / 4;
    return static_cast<uint16_t>(
            std::min<uint32_t>(std::numeric_limits<uint16_t>::max(), improved));
}

PlayerVitals::PlayerVitals(uint16_t maxHealth, uint16_t maxEnergy):
        health(maxHealth), maxHealth(maxHealth), energy(maxEnergy), maxEnergy(maxEnergy) {
    if (maxHealth == 0 || maxEnergy == 0) {
        throw std::invalid_argument(server_constants::MAXIMUM_HEALTH_AND_ENERGY_MUST_BE_GREATER_THAN_ZERO);
    }
}

uint16_t PlayerVitals::getHealth() const { return health; }

uint16_t PlayerVitals::getMaxHealth() const { return maxHealth; }

uint16_t PlayerVitals::getEnergy() const { return energy; }

uint16_t PlayerVitals::getMaxEnergy() const { return maxEnergy; }

bool PlayerVitals::hasResources() const { return health > 0 && energy > 0; }

bool PlayerVitals::hasLowHealth() const { return static_cast<uint32_t>(health) * 5 <= maxHealth; }

int PlayerVitals::movementStep(int normalStep) const {
    if (normalStep <= 0)
        return 0;
    if (hasLowHealth() || static_cast<uint32_t>(energy) * 10 <= maxEnergy) {
        return std::max(1, normalStep / 2);
    }
    return normalStep;
}

uint16_t PlayerVitals::swordDamage(uint16_t normalDamage) const {
    if (hasLowHealth() && normalDamage > 0)
        return std::max<uint16_t>(1, normalDamage / 2);
    return normalDamage;
}

void PlayerVitals::consumeEnergy(uint16_t cost) { energy = cost >= energy ? 0 : energy - cost; }

uint16_t PlayerVitals::receiveDamage(uint16_t damage, std::chrono::steady_clock::time_point now,std::chrono::milliseconds grace) {
    if (damage == 0 || now < nextDamageTime)
        return 0;
    const uint16_t applied = std::min(health, damage);
    health -= applied;
    nextDamageTime = now + grace;
    return applied;
}

void PlayerVitals::clearDamageGrace() { nextDamageTime = {}; }

void PlayerVitals::applyCombatBonus() {
    if (combatBonus)
        return;
    maxHealth = improvedMaximum(maxHealth);
    maxEnergy = improvedMaximum(maxEnergy);
    combatBonus = true;
}

void PlayerVitals::recover(bool rested) {
    health = rested ? maxHealth : std::max<uint16_t>(1, maxHealth / 2);
    energy = rested ? maxEnergy : std::max<uint16_t>(1, maxEnergy / 2);
}

void PlayerVitals::set(uint16_t newHealth, uint16_t newEnergy) {
    health = newHealth;
    energy = newEnergy;
    maxHealth = std::max(maxHealth, health);
    maxEnergy = std::max(maxEnergy, energy);
}

void PlayerVitals::restoreData(const PlayerDataLocal& saved) {
    if (saved.maxHealth == 0 || saved.maxEnergy == 0 || saved.health > saved.maxHealth ||
        saved.energy > saved.maxEnergy) {
        throw std::invalid_argument(server_constants::INVALID_SAVED_PLAYER_STATS);
    }
    health = saved.health;
    maxHealth = saved.maxHealth;
    energy = saved.energy;
    maxEnergy = saved.maxEnergy;
}
