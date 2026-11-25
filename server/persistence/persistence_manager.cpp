#include "persistence_manager.h"

#include "server_constants.h"

#include <algorithm>
#include <stdexcept>

PersistenceManager::PersistenceManager(const std::filesystem::path& directory)
    : reader(directory), writer(directory), games(reader.readIndex()) {
    for (size_t i = 0; i < games.size(); ++i) {
        for (size_t j = i + 1; j < games.size(); ++j) {
            if (games[i].id == games[j].id) {
                throw std::runtime_error(server_constants::DUPLICATE_GAME_ID_IN_PERSISTENCE_INDEX_PREFIX + games[i].id);
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

std::string PersistenceManager::mapPathFor(const std::string& gameId) const {
    for (const GameIndexDTO& entry : games) {
        if (entry.id == gameId) return entry.mapPath;
    }
    throw std::runtime_error(server_constants::GAME_IS_NOT_REGISTERED_PREFIX + gameId);
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
            throw std::runtime_error(server_constants::MISSING_PLAYER_PREVIEW_FOR_PREFIX + userName);
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
        throw std::runtime_error(server_constants::GAME_ALREADY_EXISTS_PREFIX + game.id);
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
    throw std::runtime_error(server_constants::GAME_IS_NOT_REGISTERED_PREFIX + game.id);
}

void PersistenceManager::createGame(const PersistenceDTO& game, const std::string& mapPath) {
    if (gameExists(game.id)) {
        throw std::runtime_error(server_constants::GAME_ALREADY_EXISTS_PREFIX + game.id);
    }

    if (mapPath.empty()) {
        throw std::invalid_argument(server_constants::GAME_MAP_PATH_IS_REQUIRED);
    }
    GameIndexDTO entry = indexFromGame(game);
    entry.mapPath = mapPath;

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
        throw std::runtime_error(server_constants::GAME_IS_NOT_REGISTERED_PREFIX + gameId);
    }

    return reader.readGame(gameId);
}

void PersistenceManager::saveGame(const PersistenceDTO& game) {
    for (GameIndexDTO& entry : games) {
        if (entry.id == game.id) {
            writer.writeGame(game);
            GameIndexDTO updated = indexFromGame(game);
            updated.mapPath = entry.mapPath;
            updateGameIndex(updated);
            return;
        }
    }
    throw std::runtime_error(server_constants::CANNOT_SAVE_UNREGISTERED_GAME_PREFIX + game.id);
}
