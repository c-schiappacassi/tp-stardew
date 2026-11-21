#include "dungeon_combat.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

#include "player/player.h"

#include "combat_context.h"
#include "enemy_behavior.h"
#include "projectile_combat.h"
#include "sword_combat.h"
#include "trap_combat.h"

namespace {
bool hasSkull(const DungeonRoom& room) {
    for (const auto& [id, enemy]: room.getCombat().enemies) {
        if (enemy.data.type == EnemyType::Skull) {
            return true;
        }
    }
    return false;
}
}

DungeonCombat::DungeonCombat(const EnemyConfig& config, uint32_t seed):config(config), generator(config, seed), trapRandom(seed) {}

bool DungeonCombat::initialize(std::map<uint8_t, DungeonRoom>& rooms,const std::map<uint8_t, DungeonRoomLayout>& data,EntityIdGenerator& ids) {
    if (rooms.empty() || rooms.size() != data.size()) {
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

bool DungeonCombat::regenerate(std::map<uint8_t, DungeonRoom>& rooms, EntityIdGenerator& ids) {
    if (visit == std::numeric_limits<uint32_t>::max()) {
        return false;
    }
    auto preparedRooms = rooms;
    auto preparedIds = ids;
    auto preparedGenerator = generator;
    auto preparedTrapRandom = trapRandom;
    for (auto& [number, room]: preparedRooms) {
        const auto& layout = layouts.at(number);
        if (layout.traps.size() > 256 ||
            room.getMap().getCellSize() > std::numeric_limits<uint16_t>::max()) {
            return false;
        }
        if (!preparedGenerator.populate(room, preparedIds, layout.reservedPositions) ||
            !trap_combat::populate(room, layout, config, preparedIds, preparedTrapRandom)) {
            return false;
        }
    }
    rooms = std::move(preparedRooms);
    ids = preparedIds;
    generator = std::move(preparedGenerator);
    trapRandom = preparedTrapRandom;
    ++visit;
    visitEnded = false;
    progress.newVisit();
    presence.clear();
    return true;
}

bool DungeonCombat::attack(Player& player, uint16_t id, DungeonRoom& room, Direction direction,EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) {
    CombatContext context{config, generator, progress, ids, clock, now};
    return sword_combat::attack(context, player, id, room, direction);
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

DungeonCombat::PlayersByRoom DungeonCombat::updatePresence(const std::vector<Combatant>& players,std::map<uint16_t, uint8_t> current,std::map<uint8_t, DungeonRoom>& rooms) {
    for (const auto& [id, number]: presence) {
        if (!current.contains(id)) {
            progress.leave(id, false);
        }
    }
    PlayersByRoom byRoom;
    for (const auto& actor: players) {
        if (!actor.player || actor.player->getZone() != Zone::Dungeon) {
            continue;
        }
        Player& player = *actor.player;
        if (lastVisits.contains(actor.id) && lastVisits.at(actor.id) != visit) {
            player.changeLocation(Zone::Dungeon, 1, rooms.at(1).getPassages()->entry);
        }
        lastVisits[actor.id] = visit;
        const auto number = player.getDungeonLevel();
        const auto room = rooms.find(number);
        if (room == rooms.end()) {
            continue;
        }
        if (!presence.contains(actor.id) || presence.at(actor.id) != number) {
            progress.leave(actor.id, player.canBeAttacked());
            progress.enter(actor.id, number, hasSkull(room->second));
        }
        current[actor.id] = number;
        byRoom[number].push_back(actor);
    }
    presence = std::move(current);
    return byRoom;
}

void DungeonCombat::spawnPendingAmbush(DungeonRoom& room, const std::vector<Combatant>& players,EntityIdGenerator& ids) {
    auto& pending = room.getCombat().pendingAmbushes;
    if (pending == 0) {
        return;
    }
    auto reserved = layouts.at(room.getNumber()).reservedPositions;
    for (const auto& actor: players) {
        reserved.push_back(actor.player->getPosition());
    }
    if (generator.spawnOrcs(room, ids, reserved)) {
        --pending;
    }
}

void DungeonCombat::update(CombatTime elapsed, std::map<uint8_t, DungeonRoom>& rooms,const std::vector<Combatant>& players, EntityIdGenerator& ids,std::chrono::steady_clock::time_point now) {
    if (elapsed.count() < 0) {
        throw std::invalid_argument("Time cannot move backwards");
    }
    clock += elapsed;
    const CombatTime movementTime{std::min<int64_t>(elapsed.count(), 250)};
    std::map<uint16_t, uint8_t> current;
    for (const auto& actor: players) {
        if (actor.player && actor.player->getZone() == Zone::Dungeon) {
            current.emplace(actor.id, actor.player->getDungeonLevel());
        }
    }
    if (current.empty()) {
        endVisit();
        return;
    }
    if (visitEnded && !regenerate(rooms, ids)) {
        return;
    }
    occupied = true;
    const auto byRoom = updatePresence(players, std::move(current), rooms);
    CombatContext context{config, generator, progress, ids, clock, now};
    for (const auto& [number, roomPlayers]: byRoom) {
        auto& room = rooms.at(number);
        spawnPendingAmbush(room, roomPlayers, ids);
        enemy_behavior::update(context, room, roomPlayers, movementTime);
        projectile_combat::update(context, room, roomPlayers, movementTime);
        trap_combat::update(context, room, roomPlayers);
    }
}

void DungeonCombat::onTravel(uint16_t id, uint8_t number, bool survived,const std::map<uint8_t, DungeonRoom>& rooms) {
    progress.leave(id, survived);
    presence.erase(id);
    const auto room = rooms.find(number);
    if (room == rooms.end()) {
        if (presence.empty()) {
            endVisit();
        }
        return;
    }
    progress.enter(id, number, hasSkull(room->second));
    lastVisits[id] = visit;
    presence[id] = number;
    occupied = true;
}

const CombatProgressData& DungeonCombat::getProgress() const { return progress.data(); }

bool DungeonCombat::completed() const { return progress.complete(); }

uint32_t DungeonCombat::getVisit() const { return visit; }
