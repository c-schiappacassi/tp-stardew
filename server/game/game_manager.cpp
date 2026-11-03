#include "game_manager.h"

#include <exception>
#include <mutex>
#include <string>
#include <utility>
#include <stdexcept>

std::string GameManager::createGame(const char* servicename,const GameSetup& setup) {
    std::lock_guard<std::mutex> lock(mtx);
    const std::string id = "game-" + std::to_string(nextId);

    auto game = std::make_unique<Game>(id,"",setup.worldId,servicename,setup.playerConfig,setup.dungeonConfig);
    if (!game->loadDungeon(setup.worldId, setup.layout)) {
        throw std::invalid_argument("No se pudo cargar la mazmorra de la partida");
    }

    if (!game->configureDungeonEntrance(setup.entrance)) {
        throw std::invalid_argument("La entrada de la mazmorra es invalida");
    }

    games.push_back(std::move(game));

    try {
        games.back()->start();
    } catch (const std::exception&) {
        games.pop_back();
        throw;
    }

    ++nextId;
    return id;
}


Game* GameManager::findGame(const std::string& id) {
    std::lock_guard<std::mutex> lock(mtx);

    for (const auto& game : games) {
        if (game->getId() == id) {
            return game.get();
        }
    }

    return nullptr;
}

void GameManager::stopAll() {
    std::lock_guard<std::mutex> lock(mtx);

    for (const auto& game : games) {
        game->stop();
    }

    for (const auto& game : games) {
        game->join();
    }

    games.clear();
}

GameManager::~GameManager() {
    stopAll();
}
