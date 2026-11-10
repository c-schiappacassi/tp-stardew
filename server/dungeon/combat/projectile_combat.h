#pragma once

#include <vector>

#include "dungeon/enemies/enemy.h"
#include "player/ports/combatant.h"

class DungeonRoom;
class Player;
struct CombatContext;

namespace projectile_combat {
void launch(CombatContext& context, const Enemy& enemy, const Player& target, DungeonRoom& room);
void update(CombatContext& context, DungeonRoom& room, const std::vector<Combatant>& players,CombatTime elapsed);
}
