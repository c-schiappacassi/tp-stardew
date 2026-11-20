#include "player.h"

#include <algorithm>

void Player::setVitals(uint16_t newHealth, uint16_t newEnergy) {
    health = newHealth;
    energy = newEnergy;
    maxHealth = std::max(maxHealth, health);
    maxEnergy = std::max(maxEnergy, energy);
    if (health == 0 || energy == 0) {
        passOut();
    } else if (state == PlayerState::Unconscious) {
        state = PlayerState::Idle;
        movementDirection = Direction::None;
        nextActionTime = {};
        nextDamageTime = {};
    }
}

void Player::restoreVitals() {
    health = maxHealth;
    energy = maxEnergy;
    state = PlayerState::Idle;
    movementDirection = Direction::None;
    nextActionTime = {};
    nextDamageTime = {};
}
