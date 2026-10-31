#include "dungeon.h"
#include "player.h"
#include "entity_id_generator.h"
#include <utility>

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

bool Dungeon::spawnMineral(EntityId id, uint8_t roomNumber, const Position& pos) {
    DungeonRoom* room = findRoom(roomNumber);
    if (id == 0 || room == nullptr) {
        return false;
    }

    for (const auto& [number, existingRoom] : rooms) {
        if (existingRoom.findMineral(id) != nullptr) {
            return false;
        }
    }

    const MineralDefinition* definition = chooseMineral(roomNumber);
    if (definition == nullptr) {
        return false;
    }

    return room->addMineral(
        Mineral(id, pos, definition->type, definition->hitsToBreak));
}

//esto afecta al lapislazuli por ahora nomas, los otros minerales tienen 100% de chance
std::optional<Stack> Dungeon::rollMiningReward(ResourceType type) {
    for (const auto& definition : config.minerals) {
        if (definition.type != type) {
            continue;
        }
        std::uniform_int_distribution<int> distribution(1, 100);
        const int roll = distribution(generator);

        if (roll > definition.rewardChance) {
            return std::nullopt;
        }
        return definition.reward;
    }

    return std::nullopt;
}

const DungeonConfig& Dungeon::getConfig() const {
    return config;
}

bool Dungeon::mineMineral(Player& player,EntityId mineralId,std::chrono::steady_clock::time_point now) {
    if (player.getZone() != Zone::Dungeon || !player.canUsePickaxe(now)) {
        return false;
    }

    DungeonRoom* room = findRoom(player.getDungeonLevel());
    if (room == nullptr) {
        return false;
    }

    Mineral* mineral = room->findMineral(mineralId);
    if (mineral == nullptr || mineral->isDepleted()) {
        return false;
    }

    if (!player.canReach(mineral->getPosition(), config.miningRange)) {
        return false;
    }

    if (!player.useTool(now)) {
        return false;
    }

    if (!mineral->hit()) {
        return true;
    }

    const auto reward = rollMiningReward(mineral->getType());
    if (reward.has_value()) {
        //si la recompensa no entra se pierde
        player.addItem(*reward);
    }
    //seguimos guardando el mineral roto hasta q publiquemos el estado
    return true;
}

std::map<uint8_t, std::vector<MineralData>>Dungeon::getMineralData(bool includeBroken) const {
    std::map<uint8_t, std::vector<MineralData>> result;
    for (const auto& [number, room] : rooms) {
        auto& data = result[number];
        data.reserve(room.getMinerals().size());

        for (const auto& [id, mineral] : room.getMinerals()) {
            if (!includeBroken && mineral.isDepleted()) {
                continue;
            }
            data.push_back(mineral.getPublicData());
        }
    }
    return result;
}

void Dungeon::removeBrokenMinerals() {
    for (auto& [number, room] : rooms) {
        room.removeBrokenMinerals();
    }
}

bool Dungeon::initialize(const std::map<uint8_t, DungeonRoomLayout>& layout,EntityIdGenerator& entityIds) {
    if (layout.size() != DUNGEON_ROOM_COUNT) {
        return false;
    }

    Dungeon prepared(config, 0);
    prepared.generator = generator;
    EntityIdGenerator preparedIds = entityIds;

    for (uint8_t number = 1; number <= DUNGEON_ROOM_COUNT; ++number) {
        const auto it = layout.find(number);
        if (it == layout.end()) {
            return false;
        }

        DungeonRoom* room = prepared.findRoom(number);
        if (!room->setMap(it->second.terrain)||!room->setPassages(it->second.passages)) {
            return false;
        }
    }

    for (const auto& [number, roomLayout] : layout) {
        for (const Position& pos : roomLayout.mineralSpawns) {
            if (preparedIds.getNextId() == 0) {
                return false;
            }

            if (!prepared.spawnMineral(
                    preparedIds.generate(), number, pos)) {
                return false;
            }
        }
    }
    rooms = std::move(prepared.rooms);
    generator = prepared.generator;
    entityIds = preparedIds;

    return true;
}