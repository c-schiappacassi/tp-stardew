#pragma once

#include <cstdint>

#include "dtos.h"
#include "inventory/inventory.h"
#include <chrono>
#include "player_config.h"

class Player {
private:
    std::string characterName;
    Position pos;
    PlayerState state;
    Direction dir;
    Direction movementDirection = Direction::None;
    Zone zone;
    uint8_t dungeonLevel = 0;
    uint16_t hairModel;
    uint16_t shirtModel;
    uint16_t pantsModel;
    Color hairColor;
    Color pantsColor;
    Inventory inventory;
    uint8_t currentSlot = 0;
    uint32_t gold;
    uint16_t health;
    uint16_t maxHealth;
    uint16_t energy;
    uint16_t maxEnergy;
    PlayerConfig config;
    std::chrono::steady_clock::time_point nextActionTime{};

public:
    Player(const PlayerData& initialData,uint8_t inventoryCapacity,uint16_t maxHealth,uint16_t maxEnergy,uint32_t initialGold,const PlayerConfig& config = {});

    void setMovement(Direction direction);
    void advance(int step);
    bool selectSlot(uint8_t slot);
    bool addItem(const Stack& stack);
    bool removeItem(uint8_t slot, uint16_t quantity);
    const Stack* getSelectedItem() const;
    uint8_t getSelectedSlot() const;
    bool canPerformAction(std::chrono::steady_clock::time_point now) const;
    bool performAction(uint16_t energyCost, std::chrono::milliseconds interval,std::chrono::steady_clock::time_point now);
    bool canUsePickaxe(std::chrono::steady_clock::time_point now =std::chrono::steady_clock::now()) const;
    bool canUseTool(std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now()) const;
    bool useTool(std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now());
    void updateToolState(std::chrono::steady_clock::time_point now =std::chrono::steady_clock::now());
    bool canReach(const Position& target, uint16_t range) const;
    Zone getZone() const;
    uint8_t getDungeonLevel() const;
    Position getNextPosition(int step) const;
    const Position& getPosition() const;
    CollisionBounds getCollisionBounds(const Position& position) const;
    Position getCollisionCenter() const;
    bool canAct() const;
    bool changeLocation(Zone destination,uint8_t level,const Position& target);
    void faceTarget(const Position& target);
    void faceDirection(Direction direction);
    int getMovementStep(int normalStep) const;

    PlayerData getPublicData() const;
    PlayerDataLocal getLocalData() const;
};
