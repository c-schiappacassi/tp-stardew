#pragma once

#include <cstdint>

class DungeonRoom;
class Player;
struct CombatContext;

namespace combat_damage {
void apply(CombatContext& context, Player& player, uint16_t playerId, uint16_t amount,DungeonRoom& room);
}
