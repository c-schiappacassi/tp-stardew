#pragma once

#include <cstdint>

class DungeonRoom;
class PlayerManager;
class PlayerSnapshot;
struct CombatContext;

namespace combat_damage {
void apply(CombatContext& context, uint16_t playerId, uint16_t amount, DungeonRoom& room);
}
