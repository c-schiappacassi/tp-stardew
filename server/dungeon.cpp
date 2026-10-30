#include "dungeon.h"

Dungeon::Dungeon(const DungeonConfig& config, uint32_t seed):config(config), generator(seed) {
    for (uint8_t number = 1; number <= DUNGEON_ROOM_COUNT; ++number) {
        rooms.emplace(number, DungeonRoom(number));
    }
}

DungeonRoom* Dungeon::findRoom(uint8_t number) {
    const auto it = rooms.find(number);
    return it == rooms.end() ? nullptr : &it->second;
}

const DungeonRoom* Dungeon::findRoom(uint8_t number) const {
    const auto it = rooms.find(number);
    return it == rooms.end() ? nullptr : &it->second;
}

const std::map<uint8_t, DungeonRoom>& Dungeon::getRooms() const {
    return rooms;
}

const MineralDefinition* Dungeon::chooseMineral(uint8_t roomNumber) {
    std::vector<double> weights;
    weights.reserve(config.minerals.size()); //vi que esto es para que push_back no tenga q estar redimensionando :)
    double totalWeight = 0;

    for (const auto& definition : config.minerals) {
        double weight = 0.0;
        if (roomNumber >= definition.firstRoom) {
            weight = definition.spawnWeight;
        }
        weights.push_back(weight);
        totalWeight += weight;
    }
    if (totalWeight == 0) {
        return nullptr;
    }

    std::discrete_distribution<std::size_t> distribution(
        weights.begin(), weights.end());

    return &config.minerals[distribution(generator)];
}

bool Dungeon::spawnMineral(EntityId id,uint8_t roomNumber,const Position& pos) {
    DungeonRoom* room = findRoom(roomNumber);

    if (room == nullptr || room->findMineral(id) != nullptr) {
        return false;
    }

    const MineralDefinition* definition = chooseMineral(roomNumber);
    if (definition == nullptr) {
        return false;
    }

    return room->addMineral(Mineral(id,pos,definition->type,definition->hitsToBreak));
}