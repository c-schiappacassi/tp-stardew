#pragma once
#include <cstdint>

#include "protocol_types.h"

class PlayerManager;
struct ItemTransferDTO;
struct SelectItemDTO;

class PlayerInteractions {
public:
    virtual void interact(PlayerManager& players, uint16_t playerId, Direction direction) = 0;
    virtual void depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command) = 0;
    virtual void withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command) = 0;

    virtual ~PlayerInteractions();
};
