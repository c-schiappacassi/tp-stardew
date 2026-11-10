#pragma once

#include <cstdint>

class Player;

struct Combatant {
    uint16_t id;
    Player* player;
};
