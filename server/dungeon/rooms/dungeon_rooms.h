#pragma once

#include <map>
#include <random>

#include "dungeon/dungeon_room.h"

class DungeonRooms {
private:
    std::map<uint8_t, DungeonRoom> rooms;
    std::vector<uint8_t> visitOrder;

public:
    using iterator = std::map<uint8_t, DungeonRoom>::iterator;
    using const_iterator = std::map<uint8_t, DungeonRoom>::const_iterator;

    DungeonRooms();
    DungeonRoom* find(uint8_t number);
    const DungeonRoom* find(uint8_t number) const;
    DungeonRoom& at(uint8_t number);
    const DungeonRoom& at(uint8_t number) const;
    std::size_t size() const;
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    void shuffle(std::mt19937& random);
    uint8_t firstNumber() const;
    std::optional<uint8_t> nextNumber(uint8_t number) const;

    bool canWalk(uint8_t number, const CollisionBounds& bounds) const;
    std::optional<MineData> getMineData(uint8_t number, uint32_t visit, bool includeBroken) const;
    std::optional<DungeonCombatData> getCombatData(uint8_t number, uint32_t visit) const;
    std::vector<PortalData> getPortals(uint8_t number, bool activeOnly = true) const;
    std::vector<LootData> getGroundItems(uint8_t number) const;
    std::optional<OpenChestData> openChestFor(const PlayerSnapshot& player) const;
    void removeBrokenMinerals();
};
