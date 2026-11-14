#pragma once
#include <cstdint>
#include "protocol_types.h"

class Player;
struct InteractDTO;

class PlayerInteractions {
public:
    virtual void interact(Player& player, Direction direction, uint16_t playerId = 0) = 0;

    virtual ~PlayerInteractions() = default;
};