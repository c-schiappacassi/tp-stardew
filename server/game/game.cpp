#include "game.h"
Game::Game(const std::string& id,const std::string& name,uint8_t worldId,const PlayerConfig& playerConfig,const DungeonConfig& dungeonConfig):
    id(id),
    name(name),
    worldId(worldId),
    input(std::make_shared<CommandQueue>()),
    world(dungeonConfig),
    playerManager(playerConfig, world, world, world) {}

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