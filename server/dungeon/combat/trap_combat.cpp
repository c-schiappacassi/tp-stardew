#include "trap_combat.h"

#include "dungeon/enemies/enemy_navigation.h"
#include "dungeon/rooms/dungeon_room.h"
#include "player/player.h"

#include "combat_context.h"
#include "combat_damage.h"

namespace {
bool isValid(const TrapData& trap, const DungeonRoom& room, const DungeonRoomPassages& passages) {
    const auto& map = room.getMap();
    if (!room.canWalk(CollisionBox(16, 8).at(trap.pos)) ||
        map.sameBaseCell(trap.pos, passages.entry) || map.sameBaseCell(trap.pos, passages.exit) ||
        (passages.nextRoom && map.sameBaseCell(trap.pos, *passages.nextRoom)) ||
        static_cast<uint8_t>(trap.type) > static_cast<uint8_t>(TrapType::WallFlame) ||
        trap.dir == Direction::None ||
        static_cast<uint8_t>(trap.dir) > static_cast<uint8_t>(Direction::Right)) {
        return false;
    }
    if (trap.type == TrapType::WallFlame) {
        const auto support =
                enemy_navigation::offsetPosition(trap.pos, trap.dir, -map.getCellSize());
        if (map.isWalkableBase(support)) {
            return false;
        }
    }
    return true;
}

void updateState(Trap& trap, const EnemyConfig& config, CombatTime time) {
    const int64_t warningStart = config.trapInactiveMs;
    const int64_t activeStart = warningStart + config.trapWarningMs;
    const int64_t period = activeStart + config.trapActiveMs;
    const auto phase = (time + trap.offset).count() % period;
    if (phase < warningStart) {
        trap.data.state = TrapState::Inactive;
    } else if (phase < activeStart) {
        trap.data.state = TrapState::Warning;
    } else {
        trap.data.state = TrapState::Active;
    }
}

void updateFlameRange(Trap& trap, const DungeonRoom& room, const EnemyConfig& config) {
    trap.data.range = 0;
    for (int distance = 0; distance <= config.wallFlameRange; ++distance) {
        const auto point = enemy_navigation::offsetPosition(trap.data.pos, trap.data.dir, distance);
        if (!room.canWalk(CollisionBox{16, 8}.at(point))) {
            break;
        }
        trap.data.range = static_cast<uint16_t>(distance);
    }
}

bool reachesPlayer(const Trap& trap, const Player& player, const DungeonRoom& room) {
    const auto bounds = player.getCollisionBounds(player.getPosition());
    int range = 0;
    if (trap.data.type == TrapType::WallFlame) {
        range = trap.data.range;
    }
    for (int distance = 0; distance <= range; ++distance) {
        const auto point = enemy_navigation::offsetPosition(trap.data.pos, trap.data.dir, distance);
        const auto area = CollisionBox{16, 8}.at(point);
        if (!room.canWalk(area)) {
            break;
        }
        if (area.intersects(bounds)) {
            return true;
        }
    }
    return false;
}
}

bool trap_combat::populate(DungeonRoom& room, const DungeonRoomLayout& layout,const EnemyConfig& config, EntityIdGenerator& ids) {
    std::map<uint32_t, Trap> prepared;
    auto preparedIds = ids;
    if (layout.traps.size() > 256) {
        return false;
    }
    for (auto data: layout.traps) {
        if (!isValid(data, room, layout.passages) || preparedIds.getNextId() == 0) {
            return false;
        }
        data.id = preparedIds.generate();
        data.state = TrapState::Inactive;
        data.range = 16;
        if (data.type == TrapType::WallFlame) {
            data.range = config.wallFlameRange;
        }
        Trap trap{data, CombatTime(0), {}};
        if (data.type == TrapType::WallFlame) {
            updateFlameRange(trap, room, config);
        }
        prepared.emplace(data.id, std::move(trap));
    }
    room.getCombat().traps = std::move(prepared);
    ids = preparedIds;
    return true;
}

void trap_combat::update(CombatContext& context, DungeonRoom& room,const std::vector<Combatant>& players) {
    for (auto& [id, trap]: room.getCombat().traps) {
        if (trap.data.type == TrapType::WallFlame) {
            updateFlameRange(trap, room, context.config);
        }
        updateState(trap, context.config, context.time);
        if (trap.data.state != TrapState::Active) {
            continue;
        }
        for (const auto& actor: players) {
            if (!actor.player->canBeAttacked() || context.time < trap.nextHits[actor.id]) {
                continue;
            }
            if (reachesPlayer(trap, *actor.player, room)) {
                trap.nextHits[actor.id] = context.time + CombatTime{context.config.trapHitMs};
                combat_damage::apply(context, *actor.player, actor.id, context.config.trapDamage,
                                     room);
            }
        }
    }
}
