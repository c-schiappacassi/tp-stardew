#include "trap_combat.h"

#include <algorithm>
#include <limits>

#include "dungeon/enemies/enemy_navigation.h"
#include "dungeon/rooms/dungeon_room.h"
#include "player/player_manager.h"

#include "combat_context.h"
#include "combat_damage.h"
#include "projectile_combat.h"

namespace {
CollisionBounds trapBounds(const TrapData& trap) { return CollisionBox{16, trap.type == TrapType::Spikes ? 16 : 8}.at(trap.pos); }

bool isValid(const TrapData& trap, const DungeonRoom& room, const DungeonRoomPassages& passages) {
    const auto& map = room.getMap();
    if (!room.canWalk(trapBounds(trap)) || map.sameBaseCell(trap.pos, passages.entry) || map.sameBaseCell(trap.pos, passages.exit) ||
        (passages.nextRoom && map.sameBaseCell(trap.pos, *passages.nextRoom)) || static_cast<uint8_t>(trap.type) > static_cast<uint8_t>(TrapType::WallFlame) ||
        trap.dir == Direction::None || static_cast<uint8_t>(trap.dir) > static_cast<uint8_t>(Direction::Right)) {
        return false;
    }
    if (trap.type == TrapType::WallFlame) {
        const auto support = enemy_navigation::offsetPosition(trap.pos, trap.dir, -map.getCellSize());
        if (map.isWalkableBase(support)) {
            return false;
        }
    }
    return true;
}

bool isOccupied(const TrapData& trap, const DungeonRoom& room, const std::vector<Position>& reserved, const std::vector<TrapData>& chosen) {
    const auto bounds = trapBounds(trap);
    for (const auto& position: reserved) {
        if (bounds.intersects(CollisionBox{16, 16}.at(position))) {
            return true;
        }
    }
    for (const auto& other: chosen) {
        if (bounds.intersects(trapBounds(other))) {
            return true;
        }
    }
    for (const auto& [id, enemy]: room.getCombat().enemies) {
        if (enemy.alive() && bounds.intersects(enemy.bounds(enemy.data.pos))) {
            return true;
        }
    }
    return false;
}

std::vector<TrapData> chooseTraps(const DungeonRoom& room, const DungeonRoomLayout& layout, const EnemyConfig& config, std::mt19937& random) {
    std::vector<TrapData> chosen;
    const auto positions = room.getMap().walkableBasePositions();
    // Place wall traps first so spikes cannot take the few available wall cells.
    for (const auto type: {TrapType::WallFlame, TrapType::Spikes}) {
        const auto count = type == TrapType::WallFlame ? config.wallFlamesPerRoom : config.spikesPerRoom;
        if (count == 0) {
            continue;
        }
        std::vector<TrapData> candidates;
        for (const auto& position: positions) {
            if (position.getX() > std::numeric_limits<uint16_t>::max() || position.getY() > std::numeric_limits<uint16_t>::max()) {
                continue;
            }
            for (const auto direction: {Direction::Up, Direction::Down, Direction::Left, Direction::Right}) {
                const TrapData candidate{0, position, direction, type, TrapState::Inactive, 0};
                if (!isValid(candidate, room, layout.passages) || isOccupied(candidate, room, layout.reservedPositions, chosen)) {
                    continue;
                }
                if (type == TrapType::WallFlame) {
                    const auto front = enemy_navigation::offsetPosition(position, direction, room.getMap().getCellSize());
                    if (!room.canWalk(CollisionBox{16, 8}.at(front))) {
                        continue;
                    }
                }
                candidates.push_back(candidate);
                if (type == TrapType::Spikes) {
                    break;
                }
            }
        }
        std::shuffle(candidates.begin(), candidates.end(), random);
        unsigned placed = 0;
        for (const auto& candidate: candidates) {
            if (placed >= count) {
                break;
            }
            if (!isOccupied(candidate, room, layout.reservedPositions, chosen)) {
                chosen.push_back(candidate);
                ++placed;
            }
        }
    }
    return chosen;
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

}  // namespace

bool trap_combat::populate(DungeonRoom& room, const DungeonRoomLayout& layout, const EnemyConfig& config, EntityIdGenerator& ids, std::mt19937& random) {
    std::map<uint32_t, Trap> prepared;
    auto preparedIds = ids;
    auto preparedRandom = random;
    if (layout.traps.size() > 256) {
        return false;
    }
    const auto traps = layout.traps.empty() ? chooseTraps(room, layout, config, preparedRandom) : layout.traps;
    for (auto data: traps) {
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
    random = preparedRandom;
    return true;
}

void trap_combat::update(CombatContext& context, DungeonRoom& room, const std::vector<Combatant>& players) {
    for (auto& [id, trap]: room.getCombat().traps) {
        if (trap.data.type == TrapType::WallFlame) {
            updateFlameRange(trap, room, context.config);
        }
        const auto previousState = trap.data.state;
        updateState(trap, context.config, context.time);
        if (trap.data.state != TrapState::Active) {
            continue;
        }
        if (trap.data.type == TrapType::WallFlame) {
            if (previousState != TrapState::Active) {
                projectile_combat::launch(context, trap.data, room);
            }
            continue;
        }
        for (const auto& actor: players) {
            if (!context.players.snapshot(actor.id).canBeAttacked() || context.time < trap.nextHits[actor.id]) {
                continue;
            }
            if (trapBounds(trap.data).intersects(actor.player.getHitboxBounds())) {
                trap.nextHits[actor.id] = context.time + CombatTime{context.config.trapHitMs};
                combat_damage::apply(context, actor.id, context.config.trapDamage, room);
            }
        }
    }
}
