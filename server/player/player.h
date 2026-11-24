#pragma once

#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "inventory/inventory.h"
#include "queries/player_snapshot.h"
#include "state/player_activity.h"
#include "state/player_location.h"
#include "state/player_vitals.h"

#include "dtos.h"
#include "player_config.h"

class Player {
    friend class PlayerSnapshot;

private:
    std::string characterName;
    PlayerLocation location;
    PlayerActivity activity;
    uint16_t hairModel;
    uint16_t shirtModel;
    uint16_t pantsModel;
    Color hairColor;
    Color pantsColor;
    Inventory inventory;
    uint8_t currentSlot = 0;
    uint32_t gold;
    PlayerVitals vitals;
    PlayerConfig config;

    void resetActivity();

public:
    Player(const PlayerData& initialData, uint8_t inventoryCapacity, uint16_t maxHealth,uint16_t maxEnergy, uint32_t initialGold, const PlayerConfig& config = {});

    void setMovement(Direction direction);
    void advance(int step);
    Position getNextPosition(int step) const;
    void faceTarget(const Position& target);
    void faceDirection(Direction direction);
    int getMovementStep(int normalStep) const;

    Direction getDirection() const;
    Zone getZone() const;
    uint8_t getDungeonLevel() const;
    const Position& getPosition() const;
    const std::optional<InteriorLocation>& getInterior() const;
    bool changeLocation(Zone destination, uint8_t level, const Position& target,std::optional<InteriorLocation> inside = std::nullopt);
    bool restoreLocation(Zone destination, uint8_t level, const Position& target,std::optional<InteriorLocation> inside = std::nullopt);
    bool canReach(const Position& target, uint16_t range) const;
    CollisionBounds getCollisionBounds(const Position& position) const;
    Position getCollisionCenter() const;
    CollisionBounds getHitboxBounds() const;
    Position getHitboxCenter() const;

    bool canAct() const;
    bool canBeAttacked() const;
    bool isResting() const;
    bool isSleeping() const;
    bool canUseBed() const;
    void leaveBedArea();
    void passOut();
    bool sleep();
    bool cancelSleep();
    void wakeUp();
    void setVitals(uint16_t newHealth, uint16_t newEnergy);
    void restoreVitals();

    bool canPerformAction(std::chrono::steady_clock::time_point now) const;
    bool performAction(uint16_t energyCost, std::chrono::milliseconds interval,std::chrono::steady_clock::time_point now);
    bool canUsePickaxe(std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now()) const;
    bool canUseTool(std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now()) const;
    bool useTool(std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now());
    void updateToolState(std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now());

    uint16_t receiveDamage(uint16_t damage, std::chrono::steady_clock::time_point now);
    uint16_t getSwordDamage() const;
    uint16_t getMiningPower() const;
    uint16_t getSwordRange() const;
    void applyCombatBonus();

    bool selectSlot(uint8_t slot);
    bool addItem(const Stack& stack);
    uint16_t addItemUpTo(const Stack& stack);
    bool removeItem(uint8_t slot, uint16_t quantity);
    bool removeItems(const std::vector<std::pair<uint8_t, uint16_t>>& quantities);
    const std::map<uint8_t, Stack>& getInventoryItems() const;
    const Stack* getSelectedItem() const;
    uint8_t getSelectedSlot() const;
    bool transferItemsTo(Inventory& destination, uint8_t slot, uint16_t quantity);
    bool transferItemsTo(Player& destination, uint8_t slot, uint16_t quantity);
    bool transferItemsFrom(Inventory& source, uint8_t slot, uint16_t quantity);
    bool openChest(uint32_t id);
    void closeChest();

    uint32_t addGold(uint32_t amount);
    uint32_t getGold() const;
    bool spendGold(uint32_t amount);
    bool buyItem(const Stack& stack, uint32_t price);
    bool sellItems(const std::vector<std::pair<uint8_t, uint16_t>>& quantities, uint32_t price);

    PlayerSnapshot snapshot() const;
    PlayerData getPublicData() const;
    PlayerDataLocal getLocalData() const;
    void restoreData(const PlayerDataLocal& saved);
};
