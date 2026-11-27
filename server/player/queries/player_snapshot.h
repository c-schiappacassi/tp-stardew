#pragma once

#include <chrono>
#include <optional>

#include "entities/collision_box.h"

#include "dtos.h"
#include "player/player_types.h"

class Player;

class PlayerSnapshot {
    PlayerData data;
    PlayerVitalsData vitals;
    uint32_t gold;
    uint8_t inventoryCapacity;
    uint8_t selectedSlot;
    std::optional<Stack> selected;
    CollisionBox collision;
    CollisionBox hitbox;
    std::chrono::steady_clock::time_point readyAt;
    bool active;
    bool attackable;
    bool resting;
    bool sleeping;
    bool selectedTool;
    uint16_t swordDamage;
    uint16_t swordRange;
    uint16_t miningPower;
    std::optional<uint32_t> openChestId;

public:
    PlayerSnapshot(const Player& player, const PlayerConfig& config,std::chrono::steady_clock::time_point readyAt);

    const PlayerVitalsData& getVitals() const;
    uint8_t getInventoryCapacity() const;
    const Position& getPosition() const;
    Direction getDirection() const;
    Zone getZone() const;
    uint8_t getDungeonLevel() const;
    const std::optional<InteriorLocation>& getInterior() const;
    uint32_t getGold() const;
    const Stack* getSelectedItem() const;
    uint8_t getSelectedSlot() const;
    std::optional<uint32_t> getOpenChestId() const;
    bool canAct() const;
    bool canBeAttacked() const;
    bool isResting() const;
    bool isSleeping() const;
    bool canPerformAction(std::chrono::steady_clock::time_point now) const;
    bool canUseTool(std::chrono::steady_clock::time_point now) const;
    bool canUsePickaxe(std::chrono::steady_clock::time_point now) const;
    bool canReach(const Position& target, uint16_t range) const;
    CollisionBounds getCollisionBounds(const Position& position) const;
    Position getCollisionCenter() const;
    CollisionBounds getHitboxBounds() const;
    Position getHitboxCenter() const;
    uint16_t getSwordDamage() const;
    uint16_t getSwordRange() const;
    uint16_t getMiningPower() const;
};
