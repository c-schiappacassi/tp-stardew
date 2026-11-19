#include "game.h"

#include <utility>
#include <limits>
#include <stdexcept>

Game::Game(const std::string& id,const std::string& name,uint8_t worldId,const PlayerConfig& playerConfig,const DungeonConfig& dungeonConfig,const FarmConfig& farmConfig,uint8_t maxPlayers,ChatServices chatServices,const MarketConfig& marketConfig):
    id(id),
    name(name),
    worldId(worldId),
    input(std::make_shared<CommandQueue>()),
    world(dungeonConfig, std::random_device{}(), farmConfig),
    merchants(std::make_shared<MerchantChat>(world, marketConfig)),
    playerManager(playerConfig, world, world, world, world, maxPlayers,ChatServices{chatServices.merchants ? std::move(chatServices.merchants) : merchants,std::move(chatServices.cheats)}) {}

bool Game::loadFarm(uint8_t selectedWorldId, const FarmLayout& layout) {
    std::lock_guard<std::mutex> lock(mtx);
    return !configured && world.loadFarm(selectedWorldId, layout);
}

bool Game::loadTown(const TownConfig& config) {
    std::lock_guard<std::mutex> lock(mtx);
    return !configured && world.loadTown(config);
}

std::shared_ptr<const ConstMapData> Game::mapForSession() {
    if (!constMap) constMap = std::make_shared<const ConstMapData>(world.buildConstMap());
    return constMap;
}

const std::string& Game::getId() const {
    return id;
}

std::string Game::getName() const {
    std::lock_guard<std::mutex> lock(mtx);
    return name;
}

uint8_t Game::getWorldId() const {
    std::lock_guard<std::mutex> lock(mtx);
    return worldId;
}

std::shared_ptr<CommandQueue> Game::getInputQueue() const {
    return input;
}

PersistenceDTO Game::getPersistenceData() const {
    std::lock_guard<std::mutex> lock(mtx);

    PersistenceDTO snapshot;
    snapshot.id = id;
    snapshot.farmName = name;
    snapshot.mapId = worldId;
    snapshot.currentDay = currentDay;
    snapshot.players = playerManager.getPersistencePlayers();

    for (const auto& [userName, player] : snapshot.players) {
        snapshot.users.push_back(userName);
    }

    const FarmManager& farm = world.getFarm();
    snapshot.farm = farm.getPublicData();

    for (const auto& [cropId, crop] : farm.getCrops()) {
        const auto growth = crop.getGrowthElapsed().count();
        const auto water = crop.getRemainingWaterTime().count();
        if (growth < 0 || water < 0) {
            throw std::runtime_error("Negative crop time in persistence snapshot");
        }
        snapshot.cropGrowthProgress[cropId] = static_cast<uint64_t>(growth);
        snapshot.cropWaterRemainingTime[cropId] = static_cast<uint64_t>(water);
        snapshot.harvestedCrops[cropId] = crop.isHarvested();
    }

    for (const Animal* animal : farm.getAnimals()) {
        const auto elapsed = animal->getProductionElapsed().count();
        if (elapsed < 0) {
            throw std::runtime_error("Negative animal time in persistence snapshot");
        }
        snapshot.animalProductionProgress[animal->getId()] = static_cast<uint64_t>(elapsed);
        snapshot.animalAvailableProducts[animal->getId()] = animal->getAvailableProducts();
    }

    return snapshot;
}

ServerStatus Game::buildStatus() const {
    ServerStatus status{};
    status.status = StatusCode::Ok;
    status.gameId = id;
    status.worldId = worldId;
    return status;
}

bool Game::configureDungeonEntrance(const DungeonEntrance& entrance) {
    std::lock_guard<std::mutex> lock(mtx);
    if (configured) {
        return false;
    }
    return world.configureDungeonEntrance(entrance);
}

bool Game::loadDungeon(uint8_t selectedWorldId,
        const std::map<uint8_t, DungeonRoomLayout>& layout) {
    std::lock_guard<std::mutex> lock(mtx);
    if (configured) {
        return false;
    }
    return world.loadDungeon(selectedWorldId, layout);
}
