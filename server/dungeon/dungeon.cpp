#include "dungeon.h"
#include "player/player.h"
#include "entities/entity_id_generator.h"
#include <utility>
#include <cmath>
#include <limits>

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

    DungeonRoom* room =findRoom(player.getDungeonLevel());
    if (room == nullptr) {
        return false;
    }

    Mineral* mineral = room->findMineral(mineralId);
    if (mineral == nullptr || mineral->isDepleted()) {
        return false;
    }

    if (!player.canReach(mineral->getPosition(),config.miningRange) ||!room->hasClearLine(player.getPosition(),mineral->getPosition())) {
        return false;
    }
    player.faceTarget(mineral->getPosition());

    if (!player.useTool(now)) {
        return false;
    }

    if (!mineral->hit()) {
        return true;
    }

    const auto reward =rollMiningReward(mineral->getType());
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

std::optional<MineData> Dungeon::getMineData(uint8_t roomNumber,bool includeBroken) const {
    const DungeonRoom* room = findRoom(roomNumber);
    if (!room || !room->getPassages()) {
        return std::nullopt;
    }
    MineData data;
    data.current_level = roomNumber;
    LevelData& level = data.current_level_data;
    const auto& passages = *room->getPassages();

    level.entry = passages.entry;
    level.exit = passages.exit;

    level.minerals.reserve(room->getMinerals().size());

    for (const auto& [id, mineral] : room->getMinerals()) {
        if (includeBroken || !mineral.isDepleted()) {
            level.minerals.push_back(mineral.getPublicData());
        }
    }

    return data;
}

bool Dungeon::enter(Player& player) {
    if (player.getZone() == Zone::Dungeon ||!player.canAct()) {
        return false;
    }

    const DungeonRoom* first = findRoom(1);

    if (!first ||!first->getPassages() ||!first->canWalk(first->getPassages()->entry)) {
        return false;
    }

    return player.changeLocation(Zone::Dungeon,1,first->getPassages()->entry);
}

bool Dungeon::advanceRoom(Player& player) {
    if (player.getZone() != Zone::Dungeon ||!player.canAct()) {
        return false;
    }

    const DungeonRoom* room =findRoom(player.getDungeonLevel());

    if (!room ||!room->getPassages()||!room->getPassages()->nextRoom) {
        return false;
    }

    if (!room->getMap().sameCell(player.getPosition(),*room->getPassages()->nextRoom)) {
        return false;
    }

    const DungeonRoom* next =findRoom(player.getDungeonLevel() + 1);

    if (!next ||!next->getPassages()||!next->canWalk(next->getPassages()->entry)) {
        return false;
    }

    return player.changeLocation(Zone::Dungeon,next->getNumber(),next->getPassages()->entry);
}

bool Dungeon::canExit(const Player& player) const {
    if (player.getZone() != Zone::Dungeon ||!player.canAct()) {
        return false;
    }

    const DungeonRoom* room =findRoom(player.getDungeonLevel());

    return room &&room->getPassages() &&room->getMap().sameCell(player.getPosition(),room->getPassages()->exit);
}

bool Dungeon::useSelectedItem(Player& player,std::chrono::steady_clock::time_point now) {
    if (player.getZone() != Zone::Dungeon||!player.canUsePickaxe(now)) {
        return false;
    }

    DungeonRoom* room =findRoom(player.getDungeonLevel());
    if (!room) {
        return false;
    }

    EntityId chosen = 0;
    double nearest =std::numeric_limits<double>::max();

    for (const auto& [id, mineral] : room->getMinerals()) {
        if (mineral.isDepleted()) {
            continue;
        }
        if (!player.canReach(mineral.getPosition(),config.miningRange)) {
            continue;
        }
        if (!room->hasClearLine(player.getPosition(),mineral.getPosition())) {
            continue;
        }

        const double dx =static_cast<double>(mineral.getPosition().getX()) -player.getPosition().getX();
        const double dy =static_cast<double>(mineral.getPosition().getY()) -player.getPosition().getY();
        bool ahead = false;
        switch (player.getPublicData().dir) {
            case Direction::Up:
                ahead = dy < 0 && std::abs(dx) <= -dy;
                break;

            case Direction::Down:
                ahead = dy > 0 && std::abs(dx) <= dy;
                break;

            case Direction::Left:
                ahead = dx < 0 && std::abs(dy) <= -dx;
                break;

            case Direction::Right:
                ahead = dx > 0 && std::abs(dy) <= dx;
                break;

            default:
                break;
        }
        const double distance = dx * dx + dy * dy;
        if (ahead && distance < nearest) {
            nearest = distance;
            chosen = id;
        }
    }
    return chosen != 0 &&mineMineral(player, chosen, now);
}