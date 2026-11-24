#pragma once

#include <chrono>
#include <cstdint>

struct PlayerDataLocal;

class PlayerVitals {
private:
    uint16_t health;
    uint16_t maxHealth;
    uint16_t energy;
    uint16_t maxEnergy;
    std::chrono::steady_clock::time_point nextDamageTime{};
    bool combatBonus = false;

public:
    PlayerVitals(uint16_t maxHealth, uint16_t maxEnergy);

    uint16_t getHealth() const;
    uint16_t getMaxHealth() const;
    uint16_t getEnergy() const;
    uint16_t getMaxEnergy() const;
    bool hasResources() const;
    bool hasLowHealth() const;
    int movementStep(int normalStep) const;
    uint16_t swordDamage(uint16_t normalDamage) const;

    void consumeEnergy(uint16_t cost);
    uint16_t receiveDamage(uint16_t damage, std::chrono::steady_clock::time_point now,std::chrono::milliseconds grace);
    void clearDamageGrace();
    void applyCombatBonus();
    void recover(bool rested);
    void set(uint16_t newHealth, uint16_t newEnergy);
    void restoreData(const PlayerDataLocal& saved);
};
