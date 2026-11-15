#pragma once

#include <vector>

#include "dungeon/loading/dungeon_layout.h"
#include "player/ports/combatant.h"

class DungeonRoom;
class EntityIdGenerator;
struct CombatContext;
struct EnemyConfig;

namespace trap_combat {
bool populate(DungeonRoom& room, const DungeonRoomLayout& layout, const EnemyConfig& config,EntityIdGenerator& ids);
void update(CombatContext& context, DungeonRoom& room, const std::vector<Combatant>& players);
}
