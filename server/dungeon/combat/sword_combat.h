#pragma once

#include <cstdint>

#include "protocol_types.h"

class DungeonRoom;
class PlayerManager;
class PlayerSnapshot;
struct CombatContext;

namespace sword_combat {
bool attack(CombatContext& context, uint16_t playerId, DungeonRoom& room, Direction direction);
}
