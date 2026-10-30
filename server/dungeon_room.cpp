#include "dungeon_room.h"

#include <stdexcept>
#include <utility>

DungeonRoom::DungeonRoom(uint8_t number) : number(number) {
    if (number < 1 || number > 10) {
        throw std::invalid_argument("El numero de sala debe estar entre 1 y 10");
    }
}

uint8_t DungeonRoom::getNumber() const {
    return number;
}

bool DungeonRoom::addMineral(Mineral mineral) {
    const EntityId id = mineral.getId();
    return minerals.emplace(id, std::move(mineral)).second;
}

bool DungeonRoom::removeMineral(EntityId id) {
    return minerals.erase(id) > 0;
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