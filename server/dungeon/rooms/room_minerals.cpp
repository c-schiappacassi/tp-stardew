#include "room_minerals.h"
#include <utility>

bool RoomMinerals::add(Mineral mineral, const ZoneMap& terrain) {
    const auto cell = terrain.cellAt(mineral.getPosition());
    const EntityId id = mineral.getId();

    if (!cell || !terrain.isWalkable(mineral.getPosition()) ||minerals.contains(id) || byCell.contains(*cell)) {
        return false;
    }

    const auto entry = byCell.emplace(*cell, id).first;
    try {
        minerals.emplace(id, std::move(mineral));
    } catch (...) {
        byCell.erase(entry);
        throw;
    }
    return true;
}

bool RoomMinerals::remove(EntityId id, const ZoneMap& terrain) {
    const auto it = minerals.find(id);
    if (it == minerals.end()) {
        return false;
    }

    byCell.erase(*terrain.cellAt(it->second.getPosition()));
    minerals.erase(it);
    return true;
}

Mineral* RoomMinerals::find(EntityId id) {
    const auto it = minerals.find(id);
    return it == minerals.end() ? nullptr : &it->second;
}

const Mineral* RoomMinerals::find(EntityId id) const {
    const auto it = minerals.find(id);
    return it == minerals.end() ? nullptr : &it->second;
}

const std::map<EntityId, Mineral>& RoomMinerals::all() const {
    return minerals;
}

bool RoomMinerals::blocks(const ZoneMap::Cell& cell) const {
    const auto it = byCell.find(cell);
    return it != byCell.end() && !minerals.at(it->second).isDepleted();
}

bool RoomMinerals::reindex(const ZoneMap& terrain) {
    std::map<ZoneMap::Cell, EntityId> prepared;

    for (const auto& [id, mineral] : minerals) {
        const auto cell = terrain.cellAt(mineral.getPosition());
        if (!cell || !terrain.isWalkable(mineral.getPosition()) ||
            !prepared.emplace(*cell, id).second) {
            return false;
        }
    }

    byCell = std::move(prepared);
    return true;
}

void RoomMinerals::removeBroken(const ZoneMap& terrain) {
    for (auto it = minerals.begin(); it != minerals.end();) {
        const auto current = it++;
        if (current->second.isDepleted()) {
            remove(current->first, terrain);
        }
    }
}

std::vector<MineralData> RoomMinerals::getPublicData(bool includeBroken) const {
    std::vector<MineralData> data;
    data.reserve(minerals.size());
    for (const auto& [id, mineral] : minerals) {
        if (includeBroken || !mineral.isDepleted()) {
            data.push_back(mineral.getPublicData());
        }
    }
    return data;
}