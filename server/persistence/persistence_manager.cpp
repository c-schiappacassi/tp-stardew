#include "persistence_manager.h"

#include <algorithm>
#include <stdexcept>

PersistenceManager::PersistenceManager(const std::filesystem::path& directory)
    : reader(directory), writer(directory), games(reader.readIndex()) {
    for (size_t i = 0; i < games.size(); ++i) {
        for (size_t j = i + 1; j < games.size(); ++j) {
            if (games[i].id == games[j].id) {
                throw std::runtime_error("Duplicate game ID in persistence index: " + games[i].id);
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


GameIndexDTO PersistenceManager::indexFromGame(const PersistenceDTO& game) {
    GameIndexDTO entry;
    entry.id = game.id;
    entry.farmName = game.farmName;
    entry.users = game.users;
    entry.isMultiplayer = game.users.size() > 1;
    for (const auto& [userName, player] : game.players) {
        GameIndexPlayerDTO preview;
        preview.characterName = player.characterName;
        preview.hairModel = player.hairModel;
        preview.shirtModel = player.shirtModel;
        preview.pantsModel = player.pantsModel;
        preview.hairColor = player.hairColor;
        preview.pantsColor = player.pantsColor;
        entry.playerPreviews.emplace(userName, preview);
    }
    return entry;
}

std::vector<GameInfoDTO> PersistenceManager::listGamesFor(const std::string& userName) const {
    std::vector<GameInfoDTO> result;
    for (const GameIndexDTO& game : games) {
        if (std::find(game.users.begin(), game.users.end(), userName) == game.users.end()) {
            continue;
        }
        auto it = game.playerPreviews.find(userName);
        if (it == game.playerPreviews.end()) {
            throw std::runtime_error("Missing player preview for: " + userName);
        }
        PlayerData player{};
        player.characterName = it->second.characterName;
        player.hairModel = it->second.hairModel;
        player.shirtModel = it->second.shirtModel;
        player.pantsModel = it->second.pantsModel;
        player.hairColor = it->second.hairColor;
        player.pantsColor = it->second.pantsColor;
        result.push_back(GameInfoDTO{game.id, game.farmName, player, game.isMultiplayer});
    }
    return result;
}

void PersistenceManager::registerGame(const GameIndexDTO& game) {
    if (gameExists(game.id)) {
        throw std::runtime_error("Game already exists: " + game.id);
    }
    games.push_back(game);
    try {
        saveIndex();
    } catch (...) {
        games.pop_back();
        throw;
    }
}

void PersistenceManager::updateGameIndex(const GameIndexDTO& game) {
    for (GameIndexDTO& entry : games) {
        if (entry.id == game.id) {
            GameIndexDTO previous = entry;
            entry = game;
            try {
                saveIndex();
            } catch (...) {
                entry = previous;
                throw;
            }
            return;
        }
    }
    throw std::runtime_error("Game is not registered: " + game.id);
}

void PersistenceManager::createGame(const PersistenceDTO& game) {
    if (gameExists(game.id)) {
        throw std::runtime_error("Game already exists: " + game.id);
    }

    GameIndexDTO entry = indexFromGame(game);

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
            updateGameIndex(indexFromGame(game));
            return;
        }
    }
    throw std::runtime_error("Cannot save unregistered game: " + game.id);
}
