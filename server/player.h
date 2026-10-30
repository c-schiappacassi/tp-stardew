#pragma once

#include <cstdint>

#include "dtos.h"
#include "inventory.h"

class Player {
private:
    Position pos;
    PlayerState state;
    Direction dir;
    Zone zone;
    uint8_t dungeonLevel;
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

public:
    Player(const PlayerData& initialData,uint8_t inventoryCapacity,uint16_t maxHealth,uint16_t maxEnergy,uint32_t initialGold);

    void setMovement(Direction direction);
    void advance(int step);
    bool selectSlot(uint8_t slot);
    bool addItem(const Stack& stack);
    bool removeItem(uint8_t slot, uint16_t quantity);
    const Stack* getSelectedItem() const;

    PlayerData getPublicData() const;
    PlayerDataLocal getLocalData() const;
};