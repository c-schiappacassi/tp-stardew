#include "game_manager.h"

#include <exception>
#include <mutex>
#include <string>
#include <utility>

std::string GameManager::createGame(const char* servicename) {
    std::lock_guard<std::mutex> lock(mtx);

    const std::string id = "game-" + std::to_string(nextId);
    auto game = std::make_unique<Game>(id, "", 0, servicename);
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
