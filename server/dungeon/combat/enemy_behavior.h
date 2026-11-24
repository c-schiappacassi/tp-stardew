#pragma once

#include <vector>

#include "dungeon/enemies/enemy.h"
#include "player/ports/combatant.h"

class DungeonRoom;
struct CombatContext;

namespace enemy_behavior {
void refreshVampirePairs(DungeonRoom& room);
void update(CombatContext& context, DungeonRoom& room, const std::vector<Combatant>& players, CombatTime elapsed);
} 
