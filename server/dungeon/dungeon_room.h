#pragma once

#include <cstdint>
#include <map>
#include "mineral.h"
#include "world/zone_map.h"
#include <optional>

struct DungeonRoomPassages {
    Position entry{0, 0};
    Position exit{0, 0};
    std::optional<Position> nextRoom;
};

class DungeonRoom {
private:
    uint8_t number;
    //de esta manera tenemos separado el mapa en si de lo q serian los minerales
    ZoneMap terrain;
    std::map<EntityId, Mineral> minerals;
    std::map<ZoneMap::Cell, EntityId> mineralsByCell;
    std::optional<DungeonRoomPassages> passages;
    //esto es para que puedas avanzar en la mazmorra
    bool arePassagesValid(const ZoneMap& map,const DungeonRoomPassages& data) const;
    static bool isPassageCell(const ZoneMap& map,const DungeonRoomPassages& data,const Position& pos);

public:
    explicit DungeonRoom(uint8_t number);

    uint8_t getNumber() const;
    bool addMineral(Mineral mineral);
    bool removeMineral(EntityId id);
    Mineral* findMineral(EntityId id);
    const Mineral* findMineral(EntityId id) const;
    const std::map<EntityId, Mineral>& getMinerals() const;
    bool setMap(const ZoneMap& map);
    const ZoneMap& getMap() const;
    bool canWalk(const Position& pos) const;
    void removeBrokenMinerals();
    bool setPassages(const DungeonRoomPassages& data);
    const std::optional<DungeonRoomPassages>& getPassages() const;
    bool hasClearLine(const Position& from,const Position& target) const;
};