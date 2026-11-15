#pragma once

#include <cstdint>

#include "protocol_types.h"

class DungeonRoom;
class Player;
struct CombatContext;

namespace sword_combat {
bool attack(CombatContext& context, Player& player, uint16_t playerId, DungeonRoom& room,Direction direction);
bool collectLoot(Player& player, DungeonRoom& room);
}
