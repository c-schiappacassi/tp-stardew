#include "game_manager.h"

#include <utility>

Game& GameManager::createGame(const std::string& name) {
    const std::string id = "game-" + std::to_string(nextId);
    auto game = std::make_unique<Game>(id, name);

    Game& newGame = *game;
    games.push_back(std::move(game));
    ++nextId;

    return newGame;
}

Game* GameManager::findGame(const std::string& id) {
    for (const auto& game : games) {
        if (game->getId() == id) {
            return game.get();
        }
    }

    return nullptr;
}