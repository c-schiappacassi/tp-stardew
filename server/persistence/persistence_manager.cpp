#include "persistence_manager.h"

#include <stdexcept>

PersistenceManager::PersistenceManager(const std::filesystem::path& directory)
    : reader(directory), writer(directory), games(reader.readIndex()) {
    for (size_t i = 0; i < games.size(); ++i) {
        for (size_t j = i + 1; j < games.size(); ++j) {
            if (games[i].id == games[j].id) {
                throw std::runtime_error(
                    "Duplicate game ID in persistence index: " + games[i].id);
            }
        }
    }
}

std::vector<GameIndexDTO> PersistenceManager::getGames() const {
    return games;
}

bool PersistenceManager::gameExists(const std::string& gameId) const {
    for (const GameIndexDTO& game : games) {
        if (game.id == gameId) {
            return true;
        }
    }
    return false;
}

void PersistenceManager::saveIndex() const {
    writer.writeIndex(games);
}

void PersistenceManager::createGame(const PersistenceDTO& game) {
    if (gameExists(game.id)) {
        throw std::runtime_error("Game already exists: " + game.id);
    }

    GameIndexDTO entry;
    entry.id = game.id;
    entry.farmName = game.farmName;
    entry.users = game.users;

    writer.writeGame(game);

    games.push_back(entry);

    try {
        saveIndex();
    } catch (...) {
        games.pop_back();
        throw;
    }
}

PersistenceDTO PersistenceManager::loadGame(const std::string& gameId) const {
    if (!gameExists(gameId)) {
        throw std::runtime_error("Game is not registered: " + gameId);
    }

    return reader.readGame(gameId);
}

void PersistenceManager::saveGame(const PersistenceDTO& game) {
    for (GameIndexDTO& entry : games) {
        if (entry.id == game.id) {
            writer.writeGame(game);

            if (entry.farmName != game.farmName || entry.users != game.users) {
                GameIndexDTO previous = entry;
                entry.farmName = game.farmName;
                entry.users = game.users;

                try {
                    saveIndex();
                } catch (...) {
                    entry = previous;
                    throw;
                }
            }
            return;
        }
    }

    throw std::runtime_error("Cannot save unregistered game: " + game.id);
}
