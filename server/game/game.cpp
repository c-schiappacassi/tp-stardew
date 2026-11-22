#include "game.h"

#include <utility>
#include <limits>
#include <stdexcept>

#include "community/chat/community_chat.h"
#include "persistence/community_persistence.h"

Game::Game(const std::string& id, const std::string& name, uint8_t worldId,
        const GameConfig& config, uint8_t maxPlayers, ChatServices chatServices):
    id(id),
    name(name),
    worldId(worldId),
    input(std::make_shared<CommandQueue>()),
    world(config.dungeon, std::random_device{}(), config.farm),
    merchants(std::make_shared<MerchantChat>(world, marketPrices, config.market)),
    community(config.community),
    communityRewards(community, world, marketPrices),
    communityChat(std::make_shared<CommunityChat>(community, communityRewards)),
    playerManager(config.player, world, world, world, world, maxPlayers,
        ChatServices{chatServices.merchants ? std::move(chatServices.merchants) : merchants,
                     std::move(chatServices.cheats),
                     chatServices.community ? std::move(chatServices.community) : communityChat}) {}

bool Game::loadFarm(uint8_t selectedWorldId, const FarmLayout& layout) {
    std::lock_guard<std::mutex> lock(mtx);
    return !configured && world.loadFarm(selectedWorldId, layout);
}

bool Game::loadTown(const TownConfig& config) {
    std::lock_guard<std::mutex> lock(mtx);
    return !configured && world.loadTown(config);
}

std::shared_ptr<const ConstMapData> Game::mapForSession() {
    if (!constMap) {
        ConstMapData assembled = world.buildConstMap();
        if (hasMapVisuals) {
            assembled.farm.floor = std::move(mapVisuals.farm.floor);
            assembled.farm.decor = std::move(mapVisuals.farm.decor);
            assembled.farm.frame = std::move(mapVisuals.farm.frame);
            assembled.farm.buildings = std::move(mapVisuals.farm.buildings);
            assembled.town.floor = std::move(mapVisuals.town.floor);
            assembled.town.decor = std::move(mapVisuals.town.decor);
            assembled.town.frame = std::move(mapVisuals.town.frame);
            assembled.town.buildings = std::move(mapVisuals.town.buildings);
            if (assembled.levels.size() != mapVisuals.levels.size()) {
                throw std::runtime_error("Mine visuals do not match dungeon levels");
            }
            for (std::size_t i = 0; i < assembled.levels.size(); ++i) {
                assembled.levels[i].floor = std::move(mapVisuals.levels[i].floor);
                assembled.levels[i].decor = std::move(mapVisuals.levels[i].decor);
                assembled.levels[i].frame = std::move(mapVisuals.levels[i].frame);
                assembled.levels[i].buildings = std::move(mapVisuals.levels[i].buildings);
            }
            hasMapVisuals = false;
        }
        constMap = std::make_shared<const ConstMapData>(std::move(assembled));
    }
    return constMap;
}

void Game::setMapVisuals(ConstMapData visuals) {
    std::lock_guard<std::mutex> lock(mtx);
    if (constMap || configured) {
        throw std::logic_error("Map visuals cannot be replaced after initialization");
    }
    mapVisuals = std::move(visuals);
    hasMapVisuals = true;
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

void Game::setSaveCallback(std::function<void(const PersistenceDTO&)> callback) {
    std::lock_guard<std::mutex> lock(mtx);
    saveCallback = std::move(callback);
    ++saveRequests;
}

PersistenceDTO Game::getPersistenceData() const {
    std::lock_guard<std::mutex> lock(mtx);
    return buildPersistenceData();
}

PersistenceDTO Game::buildPersistenceData() const {
    PersistenceDTO snapshot;
    snapshot.id = id;
    snapshot.farmName = name;
    snapshot.mapId = worldId;
    snapshot.currentDay = currentDay;
    snapshot.players = playerManager.getPersistencePlayers();
    snapshot.communityCenterActionProgress = CommunityPersistence::save(community, communityRewards);

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
    status.gameCompleted = community.finished();
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

void Game::restorePersistenceData(const PersistenceDTO& saved) {
    std::lock_guard<std::mutex> lock(mtx);
    if (configured || saved.id != id || saved.mapId != worldId || saved.currentDay == 0) {
        throw std::runtime_error("Cannot restore game with mismatched identity or state");
    }
    world.restoreFarm(saved);
    CommunityPersistence::restore(community, communityRewards, saved.communityCenterActionProgress);
    playerManager.restorePlayers(saved.players);
    name = saved.farmName;
    currentDay = saved.currentDay;
    configured = true;
    updateCommunity();
}
