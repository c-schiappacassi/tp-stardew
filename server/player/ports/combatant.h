#pragma once
#include <cstdint>

#include "player/queries/player_snapshot.h"

struct Combatant {
    uint16_t id;
    PlayerSnapshot player;
};
