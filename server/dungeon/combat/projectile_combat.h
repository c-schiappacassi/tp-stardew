#pragma once

#include <vector>

#include "dungeon/enemies/enemy.h"
#include "player/ports/combatant.h"

class DungeonRoom;
class PlayerManager;
class PlayerSnapshot;
struct CombatContext;

namespace projectile_combat {
void launch(CombatContext& context, const Enemy& enemy, const PlayerSnapshot& target, DungeonRoom& room);
void launch(CombatContext& context, const TrapData& trap, DungeonRoom& room);
void update(CombatContext& context, DungeonRoom& room, const std::vector<Combatant>& players, CombatTime elapsed);
}
