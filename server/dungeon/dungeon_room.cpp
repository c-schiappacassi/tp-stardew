#include "dungeon_room.h"

#include <stdexcept>
#include <utility>
#include <algorithm>
#include <cmath>
#include "dungeon_config.h"

DungeonRoom::DungeonRoom(uint8_t number) : number(number) {
    if (number < 1 || number > DUNGEON_ROOM_COUNT) {
        throw std::invalid_argument("El numero de sala debe estar entre 1 y 10");
    }
}

uint8_t DungeonRoom::getNumber() const {
    return number;
}

bool DungeonRoom::addMineral(Mineral mineral) {
    const auto cell = terrain.cellAt(mineral.getPosition());

    if (!cell.has_value() ||
        !terrain.isWalkable(mineral.getPosition())) {
        return false;
    }

    if (passages.has_value() && isPassageCell(terrain, *passages, mineral.getPosition())) {
        return false;
    }

    const EntityId id = mineral.getId();
    if (minerals.contains(id) || mineralsByCell.contains(*cell)) {
        return false;
    }

    auto indexEntry = mineralsByCell.emplace(*cell, id).first;
    try {
        minerals.emplace(id, std::move(mineral));
    } catch (...) {
        mineralsByCell.erase(indexEntry);
        throw;
    }

    return true;
}

bool DungeonRoom::removeMineral(EntityId id) {
    const auto it = minerals.find(id);

    if (it == minerals.end()) {
        return false;
    }

    const auto cell = terrain.cellAt(it->second.getPosition());
    mineralsByCell.erase(*cell);
    minerals.erase(it);

    return true;
}

Mineral* DungeonRoom::findMineral(EntityId id) {
    const auto it = minerals.find(id);
    return it == minerals.end() ? nullptr : &it->second;
}

const Mineral* DungeonRoom::findMineral(EntityId id) const {
    const auto it = minerals.find(id);
    return it == minerals.end() ? nullptr : &it->second;
}

const std::map<EntityId, Mineral>& DungeonRoom::getMinerals() const {
    return minerals;
}

bool DungeonRoom::setMap(const ZoneMap& map) {
    if (map.isEmpty()) {
        return false;
    }
    if (passages.has_value() &&
        !arePassagesValid(map, *passages)) {
        return false;
    }

    std::map<ZoneMap::Cell, EntityId> newIndex;

    for (const auto& [id, mineral] : minerals) {
        const auto cell = map.cellAt(mineral.getPosition());

        if (!cell.has_value() ||
            !map.isWalkable(mineral.getPosition())) {
            return false;
        }
        if (passages.has_value() &&
            isPassageCell(map, *passages, mineral.getPosition())) {
            return false;
        }
        if (!newIndex.emplace(*cell, id).second) {
            return false;
        }
    }

    ZoneMap newTerrain = map;
    terrain = std::move(newTerrain);
    mineralsByCell = std::move(newIndex);

    return true;
}

const ZoneMap& DungeonRoom::getMap() const {
    return terrain;
}

bool DungeonRoom::canWalk(const Position& pos) const {
    if (!terrain.isWalkable(pos)) {
        return false;
    }

    const auto cell = terrain.cellAt(pos);
    const auto it = mineralsByCell.find(*cell);
    if (it == mineralsByCell.end()) {
        return true;
    }

    return minerals.at(it->second).isDepleted();
}

void DungeonRoom::removeBrokenMinerals() {
    auto it = minerals.begin();

    while (it != minerals.end()) {
        if (it->second.isDepleted()) {
            const EntityId id = it->first;
            ++it;
            removeMineral(id);
        } else {
            ++it;
        }
    }
}

bool DungeonRoom::isPassageCell(const ZoneMap& map,const DungeonRoomPassages& data,const Position& pos) {
    if (map.sameCell(pos, data.entry) || map.sameCell(pos, data.exit)) {
        return true;
    }

    return data.nextRoom.has_value() && map.sameCell(pos, *data.nextRoom);
}

bool DungeonRoom::arePassagesValid( const ZoneMap& map, const DungeonRoomPassages& data) const {
    if (!map.isWalkable(data.entry) || !map.isWalkable(data.exit)) {
        return false;
    }

    if (number < DUNGEON_ROOM_COUNT && !data.nextRoom.has_value()) {
        return false;
    }

    if (number == DUNGEON_ROOM_COUNT && data.nextRoom.has_value()) {
        return false;
    }

    if (data.nextRoom.has_value()) {
        if (!map.isWalkable(*data.nextRoom) ||map.sameCell(data.entry, *data.nextRoom) ||map.sameCell(data.exit, *data.nextRoom)) {
            return false;
        }
    }

    return map.hasPath(data.entry, data.exit) && (!data.nextRoom || map.hasPath(data.entry, *data.nextRoom));
}

bool DungeonRoom::setPassages(const DungeonRoomPassages& data) {
    if (!arePassagesValid(terrain, data)) {
        return false;
    }

    for (const auto& [id, mineral] : minerals) {
        if (isPassageCell(terrain, data, mineral.getPosition())) {
            return false;
        }
    }
    passages = data;
    return true;
}

const std::optional<DungeonRoomPassages>&DungeonRoom::getPassages() const {
    return passages;
}

//con esto hacemos que si el player quiere romper algo que entre medio tiene una pared o piedra no puede por mas que le de la distancia
bool DungeonRoom::hasClearLine(const Position& from,const Position& target) const {
    if (!canWalk(from) || !terrain.isWalkable(target)) {
        return false;
    }

    const double dx =static_cast<double>(target.getX()) - from.getX();
    const double dy =static_cast<double>(target.getY()) - from.getY();
    const int steps = static_cast<int>(std::max(std::abs(dx), std::abs(dy)));

    if (steps == 0) {
        return true;
    }

    Position previous = from;

    for (int step = 1; step <= steps; ++step) {
        const Position sample(static_cast<int>(std::round(from.getX() + dx * step / steps)),static_cast<int>(std::round(from.getY() + dy * step / steps)));
        const auto previousCell = terrain.cellAt(previous);
        const auto sampleCell = terrain.cellAt(sample);

        if (!previousCell || !sampleCell) {
            return false;
        }

        const bool crossesCorner =previousCell->first != sampleCell->first &&previousCell->second != sampleCell->second;
        //controlamos las esquinas
        if (crossesCorner) {
            const Position horizontal(sample.getX(), previous.getY());
            const Position vertical(previous.getX(), sample.getY());
            if (!canWalk(horizontal) || !canWalk(vertical)) {
                return false;
            }
        }

        if (terrain.sameCell(sample, target)) {
            return true;
        }
        if (!canWalk(sample)) {
            return false;
        }

        previous = sample;
    }

    return true;
}