#include "dungeon_room.h"

#include <stdexcept>
#include <utility>
#include "dungeon/dungeon_config.h"

DungeonRoom::DungeonRoom(uint8_t number) : number(number) {
    if (number < 1 || number > DUNGEON_ROOM_COUNT) {
        throw std::invalid_argument("El numero de sala debe estar entre 1 y 10");
    }
}

uint8_t DungeonRoom::getNumber() const {
    return number;
}

bool DungeonRoom::addMineral(Mineral mineral) {
    if (passages && geometry.isPassageCell(terrain, *passages, mineral.getPosition())) {
        return false;
    }
    return minerals.add(std::move(mineral), terrain);
}

bool DungeonRoom::removeMineral(EntityId id) {
    return minerals.remove(id, terrain);
}

Mineral* DungeonRoom::findMineral(EntityId id) {
    return minerals.find(id);
}

const Mineral* DungeonRoom::findMineral(EntityId id) const {
    return minerals.find(id);
}

const std::map<EntityId, Mineral>& DungeonRoom::getMinerals() const {
    return minerals.all();
}

const ZoneMap& DungeonRoom::getMap() const {
    return terrain;
}

bool DungeonRoom::canWalk(const Position& pos) const {
    return geometry.canWalk(terrain, minerals, pos);
}

void DungeonRoom::removeBrokenMinerals() {
    minerals.removeBroken(terrain);
}

bool DungeonRoom::setPassages(const DungeonRoomPassages& data) {
    if (!geometry.arePassagesValid(number, terrain, data)) {
        return false;
    }

    for (const auto& [id, mineral] : minerals.all()) {
        if (geometry.isPassageCell(terrain, data, mineral.getPosition())) {
            return false;
        }
    }
    passages = data;
    return true;
}

const std::optional<DungeonRoomPassages>& DungeonRoom::getPassages() const {
    return passages;
}

bool DungeonRoom::hasClearLine(const Position& from, const Position& target) const {
    return geometry.hasClearLine(terrain, minerals, from, target);
}

std::optional<MineData> DungeonRoom::getMineData(bool includeBroken) const {
    if (!passages) {
        return std::nullopt;
    }
    return MineData{number, LevelData{passages->entry, passages->exit,minerals.getPublicData(includeBroken)}};
}

bool DungeonRoom::setMap(const ZoneMap& map) {
    if (map.isEmpty() || (passages && !geometry.arePassagesValid(number, map, *passages))) {
        return false;
    }
    if (passages) {
        for (const auto& [id, mineral] : minerals.all()) {
            if (geometry.isPassageCell(map, *passages, mineral.getPosition())) {
                return false;
            }
        }
    }
    ZoneMap prepared = map;
    if (!minerals.reindex(prepared)) {
        return false;
    }
    terrain = std::move(prepared);
    return true;
}