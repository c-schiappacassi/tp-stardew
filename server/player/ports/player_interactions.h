#pragma once
#include <cstdint>

#include "protocol_types.h"

class PlayerManager;

class PlayerInteractions {
public:
    virtual void interact(PlayerManager& players, uint16_t playerId, Direction direction) = 0;

    virtual ~PlayerInteractions();
};
