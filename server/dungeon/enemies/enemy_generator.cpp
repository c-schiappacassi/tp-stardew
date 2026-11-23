#include "enemy_generator.h"

#include <algorithm>

#include "dungeon/rooms/dungeon_room.h"

#include "enemy_navigation.h"

EnemyGenerator::EnemyGenerator(const EnemyConfig& config, uint32_t seed):
        config(config), random(seed) {
    config.validate();
}

bool EnemyGenerator::roll(uint8_t percent) {
    return std::uniform_int_distribution<int>{1, 100}(random) <= percent;
}

uint16_t EnemyGenerator::gold() {
    return std::uniform_int_distribution<uint16_t>{config.goldMin, config.goldMax}(random);
}

namespace {
bool isSpawnPositionValid(const DungeonRoom& room, const Position& position,const std::vector<Position>& reserved,const std::map<uint32_t, Enemy>& enemies, uint16_t safetyRange) {
    const auto bounds = CollisionBox(12, 8).at(position);
    const auto& passages = *room.getPassages();
    const double squaredSafetyRange = static_cast<double>(safetyRange) * safetyRange;
    if (!room.canWalk(bounds) ||
        enemy_navigation::squaredDistance(position, passages.entry) < squaredSafetyRange ||
        room.getMap().sameBaseCell(position, passages.exit) ||
        std::any_of(passages.extraExits.begin(), passages.extraExits.end(),
                    [&](const Position& door) { return room.getMap().sameBaseCell(position, door); }) ||
        (passages.nextRoom && room.getMap().sameBaseCell(position, *passages.nextRoom))) {
        return false;
    }
    for (const auto& other: reserved) {
        if (bounds.intersects(CollisionBox(16, 16).at(other))) {
            return false;
        }
    }
    for (const auto& [id, enemy]: enemies) {
        if (enemy.alive() && bounds.intersects(enemy.bounds(enemy.data.pos))) {
            return false;
        }
    }
    const auto route = enemy_navigation::nextWaypoint(room, passages.entry, position);
    return route.has_value();
}
}

bool EnemyGenerator::spawn(DungeonRoom& room, const std::vector<EnemyType>& types,EntityIdGenerator& ids, const std::vector<Position>& reserved,uint32_t group, uint32_t pair) {
    if (!room.getPassages() ||
        room.getCombat().enemies.size() + types.size() > config.maxEnemiesPerRoom) {
        return false;
    }
    auto candidates = room.getMap().walkableBasePositions();
    std::shuffle(candidates.begin(), candidates.end(), random);
    auto prepared = room.getCombat().enemies;
    auto preparedIds = ids;
    for (auto type: types) {
        std::optional<Position> position;
        for (const auto& candidate: candidates) {
            if (isSpawnPositionValid(room, candidate, reserved, prepared,config.spawnSafetyRange)) {
                position = candidate;
                break;
            }
        }
        if (!position || preparedIds.getNextId() == 0) {
            return false;
        }
        Enemy enemy(preparedIds.generate(), type, *position, config.definitions.at(type));
        enemy.groupId = group;
        enemy.pairId = pair;
        prepared.emplace(enemy.data.id, std::move(enemy));
    }
    room.getCombat().enemies = std::move(prepared);
    ids = preparedIds;
    return true;
}

bool EnemyGenerator::spawnOrcs(DungeonRoom& room, EntityIdGenerator& ids,const std::vector<Position>& reserved, int minimum) {
    if (minimum < 3 || minimum > 6) {
        return false;
    }
    auto preparedIds = ids;
    if (preparedIds.getNextId() == 0) {
        return false;
    }
    const auto group = preparedIds.generate();
    auto preparedGenerator = *this;
    const int count = std::uniform_int_distribution<int>(minimum, 6)(preparedGenerator.random);
    if (!preparedGenerator.spawn(room, std::vector<EnemyType>(count, EnemyType::Orc), preparedIds,reserved, group, 0)) {
        return false;
    }
    ids = preparedIds;
    random = preparedGenerator.random;
    return true;
}

bool EnemyGenerator::spawnEnemies(DungeonRoom& room, EnemyType type, uint16_t quantity,
                                  bool pairs, EntityIdGenerator& ids,
                                  const std::vector<Position>& reserved) {
    const uint32_t count = static_cast<uint32_t>(quantity) * (pairs ? 2 : 1);
    if (quantity == 0 || type > EnemyType::Skull ||
        (pairs && type != EnemyType::VampireMale) ||
        room.getCombat().enemies.size() + count > config.maxEnemiesPerRoom) return false;

    auto preparedRoom = room;
    auto preparedIds = ids;
    auto preparedGenerator = *this;
    if (pairs) {
        for (uint16_t index = 0; index < quantity; ++index) {
            if (preparedIds.getNextId() == 0) return false;
            const auto pair = preparedIds.generate();
            if (!preparedGenerator.spawn(preparedRoom,
                    {EnemyType::VampireMale, EnemyType::VampireFemale}, preparedIds,
                    reserved, 0, pair)) return false;
        }
    } else {
        uint32_t group = 0;
        if (type == EnemyType::Orc) {
            if (preparedIds.getNextId() == 0) return false;
            group = preparedIds.generate();
        }
        if (!preparedGenerator.spawn(preparedRoom, std::vector<EnemyType>(quantity, type),
                                     preparedIds, reserved, group, 0)) return false;
    }
    room.getCombat() = std::move(preparedRoom.getCombat());
    ids = preparedIds;
    random = preparedGenerator.random;
    return true;
}

bool EnemyGenerator::populate(DungeonRoom& room, EntityIdGenerator& ids,const std::vector<Position>& reserved) {
    auto preparedRoom = room;
    auto preparedIds = ids;
    auto preparedGenerator = *this;
    if (!preparedGenerator.populateRoom(preparedRoom, preparedIds, reserved)) {
        return false;
    }
    room.getCombat() = std::move(preparedRoom.getCombat());
    ids = preparedIds;
    random = preparedGenerator.random;
    return true;
}

bool EnemyGenerator::populateRoom(DungeonRoom& room, EntityIdGenerator& ids,const std::vector<Position>& reserved) {
    room.getCombat() = RoomCombat();
    if (!spawn(room, std::vector<EnemyType>(config.skeletonsPerRoom, EnemyType::Skeleton), ids,reserved, 0, 0)) {
        return false;
    }
    const auto number = room.getNumber();
    if (config.orcGroupEvery && number % config.orcGroupEvery == 0 &&
        !spawnOrcs(room, ids, reserved, 4)) {
        return false;
    }
    if (config.vampirePairEvery && number % config.vampirePairEvery == 0) {
        if (ids.getNextId() == 0) {
            return false;
        }
        const auto pair = ids.generate();
        if (!spawn(room, {EnemyType::VampireMale, EnemyType::VampireFemale}, ids, reserved, 0,pair)) {
            return false;
        }
    }
    if (config.skullEvery && number % config.skullEvery == 0 &&
        !spawn(room, {EnemyType::Skull}, ids, reserved, 0, 0)) {
        return false;
    }
    return true;
}
