#pragma once

#include <map>
#include "dungeon/mining/mineral.h"
#include "world/zone_map.h"

class RoomMinerals {
private:
    std::map<EntityId, Mineral> minerals;
    std::map<ZoneMap::Cell, EntityId> byCell;

public:
    bool add(Mineral mineral, const ZoneMap& terrain);
    bool remove(EntityId id, const ZoneMap& terrain);
    Mineral* find(EntityId id);
    const Mineral* find(EntityId id) const;
    const std::map<EntityId, Mineral>& all() const;
    bool blocks(const ZoneMap::Cell& cell) const;
    bool reindex(const ZoneMap& terrain);
    void removeBroken(const ZoneMap& terrain);
    std::vector<MineralData> getPublicData(bool includeBroken) const;
};