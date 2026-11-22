#pragma once

#include <cstdint>

class DungeonRoom;
class Enemy;
struct CombatContext;

namespace enemy_defeat {
void reward(CombatContext& context, uint16_t playerId, const Enemy& enemy, DungeonRoom& room);
}
