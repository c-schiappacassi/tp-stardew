#include "dungeon_combat.h"

#include "server_constants.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

#include "dungeon/enemies/enemy_navigation.h"
#include "dungeon/rooms/dungeon_rooms.h"
#include "player/player_manager.h"

#include "combat_context.h"
#include "enemy_behavior.h"
#include "enemy_defeat.h"
#include "projectile_combat.h"
#include "sword_combat.h"
#include "trap_combat.h"

namespace {
bool hasSkull(const DungeonRoom& room) {
    for (const auto& [id, enemy]: room.getCombat().enemies) {
        if (enemy.getType() == EnemyType::Skull) {
            return true;
        }
    }
    return false;
}
}  // namespace

DungeonCombat::DungeonCombat(const EnemyConfig& config, uint32_t seed): config(config), generator(config, seed), trapRandom(seed), routeRandom(seed) {}

bool DungeonCombat::initialize(DungeonRooms& rooms, const std::map<uint8_t, DungeonRoomLayout>& data, EntityIdGenerator& ids) {
    if (rooms.size() == 0 || rooms.size() != data.size()) {
        return false;
    }
    for (const auto& [number, room]: rooms) {
        if (!data.contains(number) || number != room.getNumber() || !room.getPassages()) {
            return false;
        }
    }
    auto prepared = *this;
    prepared.layouts = data;
    if (!prepared.regenerate(rooms, ids)) {
        return false;
    }
    *this = std::move(prepared);
    return true;
}

bool DungeonCombat::regenerate(DungeonRooms& rooms, EntityIdGenerator& ids) {
    if (visit == std::numeric_limits<uint32_t>::max()) {
        return false;
    }
    auto preparedRooms = rooms;
    auto preparedIds = ids;
    auto preparedGenerator = generator;
    auto preparedTrapRandom = trapRandom;
    auto preparedRouteRandom = routeRandom;
    preparedRooms.shuffle(preparedRouteRandom);
    for (auto& [number, room]: preparedRooms) {
        const auto& layout = layouts.at(number);
        if (layout.traps.size() > 256 || room.getMap().getCellSize() > std::numeric_limits<uint16_t>::max()) {
            return false;
        }
        if (!preparedGenerator.populate(room, preparedIds, layout.reservedPositions) || !trap_combat::populate(room, layout, config, preparedIds, preparedTrapRandom)) {
            return false;
        }
    }
    rooms = std::move(preparedRooms);
    ids = preparedIds;
    generator = std::move(preparedGenerator);
    trapRandom = preparedTrapRandom;
    routeRandom = preparedRouteRandom;
    ++visit;
    visitEnded = false;
    progress.newVisit();
    presence.clear();
    return true;
}

bool DungeonCombat::prepareEntry(const PlayerManager& players, DungeonRooms& rooms, EntityIdGenerator& ids) {
    const auto combatants = players.combatants();
    if (std::any_of(combatants.begin(), combatants.end(), [](const auto& actor) { return actor.player.getZone() == Zone::Dungeon; }))
        return true;
    // A disconnect and the next entrance can arrive in the same command batch.
    endVisit();
    return !visitEnded || regenerate(rooms, ids);
}

bool DungeonCombat::attack(PlayerManager& players, uint16_t playerId, DungeonRoom& room, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) {
    CombatContext context{players, config, generator, progress, ids, clock, now};
    return sword_combat::attack(context, playerId, room, direction);
}

void DungeonCombat::silverMined(DungeonRoom& room) {
    auto& pending = room.getCombat().pendingAmbushes;
    if (generator.roll(config.miningAmbushChance) && pending < 8) {
        ++pending;
    }
}

void DungeonCombat::endVisit() {
    for (const auto& [id, number]: presence) {
        progress.leave(id, false);
    }
    presence.clear();
    if (occupied) {
        visitEnded = true;
    }
    occupied = false;
}

DungeonCombat::PlayersByRoom DungeonCombat::updatePresence(
    const std::vector<Combatant>& combatants, PlayerManager& players, std::map<uint16_t, uint8_t> current, DungeonRooms& rooms) {
    for (const auto& [id, number]: presence) {
        if (!current.contains(id)) {
            progress.leave(id, false);
        }
    }
    PlayersByRoom byRoom;
    for (const auto& actor: combatants) {
        if (actor.player.getZone() != Zone::Dungeon) {
            continue;
        }
        auto player = actor.player;
        if (lastVisits.contains(actor.id) && lastVisits.at(actor.id) != visit) {
            const auto first = rooms.firstNumber();
            players.apply(actor.id, player_action::ChangeLocation{Zone::Dungeon, first, rooms.at(first).getPassages()->entry});
            player = players.snapshot(actor.id);
        }
        lastVisits[actor.id] = visit;
        const auto number = player.getDungeonLevel();
        const auto* room = rooms.find(number);
        if (!room) {
            continue;
        }
        if (!presence.contains(actor.id) || presence.at(actor.id) != number) {
            progress.leave(actor.id, player.canBeAttacked());
            progress.enter(actor.id, number, hasSkull(*room));
        }
        current[actor.id] = number;
        byRoom[number].push_back({actor.id, std::move(player)});
    }
    presence = std::move(current);
    return byRoom;
}

void DungeonCombat::spawnPendingAmbush(DungeonRoom& room, const std::vector<Combatant>& combatants, EntityIdGenerator& ids) {
    auto& pending = room.getCombat().pendingAmbushes;
    if (pending == 0) {
        return;
    }
    auto reserved = layouts.at(room.getNumber()).reservedPositions;
    for (const auto& actor: combatants) {
        reserved.push_back(actor.player.getPosition());
    }
    if (generator.spawnOrcs(room, ids, reserved)) {
        --pending;
    }
}

void DungeonCombat::update(
    CombatTime elapsed, DungeonRooms& rooms, const std::vector<Combatant>& combatants, PlayerManager& players, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) {
    if (elapsed.count() < 0) {
        throw std::invalid_argument(server_constants::TIME_CANNOT_MOVE_BACKWARDS);
    }
    clock += elapsed;
    const CombatTime movementTime{std::min<int64_t>(elapsed.count(), 250)};
    std::map<uint16_t, uint8_t> current;
    for (const auto& actor: combatants) {
        if (actor.player.getZone() == Zone::Dungeon) {
            current.emplace(actor.id, actor.player.getDungeonLevel());
        }
    }
    if (current.empty()) {
        endVisit();
        return;
    }

    occupied = true;
    const auto byRoom = updatePresence(combatants, players, std::move(current), rooms);
    CombatContext context{players, config, generator, progress, ids, clock, now};
    for (const auto& [number, roomPlayers]: byRoom) {
        auto& room = rooms.at(number);
        spawnPendingAmbush(room, roomPlayers, ids);
        enemy_behavior::update(context, room, roomPlayers, movementTime);
        projectile_combat::update(context, room, roomPlayers, movementTime);
        trap_combat::update(context, room, roomPlayers);
    }
}

void DungeonCombat::onTravel(uint16_t id, uint8_t number, bool survived, const DungeonRooms& rooms) {
    progress.leave(id, survived);
    presence.erase(id);
    const auto* room = rooms.find(number);
    if (!room) {
        if (presence.empty()) {
            endVisit();
        }
        return;
    }
    progress.enter(id, number, hasSkull(*room));
    lastVisits[id] = visit;
    presence[id] = number;
    occupied = true;
}

const CombatProgressData& DungeonCombat::getProgress() const { return progress.data(); }

bool DungeonCombat::completed() const { return progress.complete(); }

uint32_t DungeonCombat::getVisit() const { return visit; }

bool DungeonCombat::spawnEnemies(DungeonRoom& room, EnemyType type, uint16_t quantity, bool pairs, EntityIdGenerator& ids, const std::vector<Position>& occupied) {
    const auto layout = layouts.find(room.getNumber());
    if (layout == layouts.end())
        return false;
    auto reserved = layout->second.reservedPositions;
    reserved.insert(reserved.end(), occupied.begin(), occupied.end());
    if (!generator.spawnEnemies(room, type, quantity, pairs, ids, reserved))
        return false;
    if (type == EnemyType::Skull) {
        for (const auto& [id, number]: presence) {
            if (number == room.getNumber())
                progress.observeSkull(id, number);
        }
    }
    return true;
}

uint16_t DungeonCombat::killEnemies(PlayerManager& players, uint16_t playerId, DungeonRoom& room, bool nearestOnly, EntityIdGenerator& ids) {
    const auto player = players.snapshot(playerId);
    Enemy* nearest = nullptr;
    if (nearestOnly) {
        double distance = std::numeric_limits<double>::max();
        for (auto& [id, enemy]: room.getCombat().enemies) {
            const auto candidate = enemy_navigation::squaredDistance(player.getPosition(), enemy.getPosition());
            if (enemy.alive() && candidate < distance) {
                nearest = &enemy;
                distance = candidate;
            }
        }
    }
    CombatContext context{players, config, generator, progress, ids, clock, std::chrono::steady_clock::now()};
    uint16_t killed = 0;
    for (auto& [id, enemy]: room.getCombat().enemies) {
        if ((!nearestOnly || &enemy == nearest) && enemy.kill(clock, config)) {
            enemy_defeat::reward(context, playerId, enemy, room);
            ++killed;
        }
    }
    enemy_behavior::refreshVampirePairs(room);
    return killed;
}

bool DungeonCombat::spawnEnemies(const PlayerManager& players, uint16_t playerId, DungeonRooms& rooms, EnemyType type, uint16_t quantity, bool pairs, EntityIdGenerator& ids,
    const std::vector<Position>& occupiedPositions) {
    const auto player = players.snapshot(playerId);
    if (player.getZone() != Zone::Dungeon || !player.canAct())
        return false;
    auto* room = rooms.find(player.getDungeonLevel());
    if (!room)
        return false;
    auto reserved = occupiedPositions;
    reserved.push_back(player.getPosition());
    return spawnEnemies(*room, type, quantity, pairs, ids, reserved);
}

uint16_t DungeonCombat::killEnemies(PlayerManager& players, uint16_t playerId, DungeonRooms& rooms, bool nearestOnly, EntityIdGenerator& ids) {
    const auto player = players.snapshot(playerId);
    if (player.getZone() != Zone::Dungeon)
        return 0;
    auto* room = rooms.find(player.getDungeonLevel());
    return room ? killEnemies(players, playerId, *room, nearestOnly, ids) : 0;
}
